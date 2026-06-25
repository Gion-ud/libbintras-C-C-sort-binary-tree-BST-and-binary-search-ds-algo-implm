// bintras_*: binary, trees and sort
// libbintras

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <intdef.h>
#include <bintras_bst.h>
#include "dbg_print.h"

typedef struct _BSTNodeFreeChunk {
    BSTNode *next_free_np;
} _BSTNodeFreeChunk;

static_assert(
    sizeof(BSTNode) >= sizeof(_BSTNodeFreeChunk),
    "sizeof(BSTNode) must be at least sizeof(_BSTNodeFreeChunk)"
);

typedef struct _BSTNodePool {
    BSTNode    *node_arr;
    BSTNode    *free_head_np;    
    size_t      count;
    size_t      capacity;
} _BSTNodePool;

struct bintras_bst {
    _BSTNodePool    node_pool;      // [0]; ownership
    uint8_t        *state_arr;      // [1]; ownership
    BSTNode        *header_np;      // [2]; non owning ref; header
    size_t          dead_count;     // [3]; dead node count
    int (*cmp_func)(void*, void*);
};
typedef bintras_bst BST;

/*
 * bst.node_header.parent   = root
 * bst.node_header.left     = min
 * bst.node_header.right    = max
 * bst.node_header.data     = (bst.is_modified) ? bst.node_header : null
 */

#define BST_CAPACITY_DEFAULT 32u
#define BST_NODE_EMPTY  0u
#define BST_NODE_ALIVE  1u
#define BST_NODE_DEAD   2u

static BSTNode *_bstpool_init(_BSTNodePool *pool_p, size_t capacity) {
    _dbg_log_msg("begin");
    assert(pool_p);
    assert(capacity);
    memset(pool_p, 0, sizeof(*pool_p));

    size_t real_capacity = capacity + 1;
    pool_p->node_arr = (BSTNode*)malloc(real_capacity * sizeof(BSTNode));
    if (!pool_p->node_arr) return NULL;
    memset(pool_p->node_arr, 0, real_capacity * sizeof(BSTNode));

    for (size_t i = 0ul; i < capacity; ++i) {
        __auto_type __chunk_p = (_BSTNodeFreeChunk*)&pool_p->node_arr[i];
        __chunk_p->next_free_np = &pool_p->node_arr[i + 1];
    }
    __auto_type __chunk_p = (_BSTNodeFreeChunk*)&pool_p->node_arr[capacity - 1];
    __chunk_p->next_free_np = NULL;

    pool_p->free_head_np    = &pool_p->node_arr[0];
    pool_p->count           = 0;
    pool_p->capacity        = capacity;

    _dbg_log_msg("end");
    return &pool_p->node_arr[real_capacity - 1]; // tail is reserved for BST header node
}

static void _bstpool_fini(_BSTNodePool *pool_p) {
    _dbg_log_msg("begin");
    if (!pool_p) return;
    if (pool_p->node_arr) free(pool_p->node_arr);
    pool_p->free_head_np    = NULL;
    pool_p->count           = 0;
    pool_p->capacity        = 0;
    _dbg_log_msg("end");
}

static BSTNode *_bstpool_alloc_node(_BSTNodePool *pool_p) {
    assert(pool_p);
    assert(pool_p->node_arr);
    assert(pool_p->capacity);
    if (pool_p->count == pool_p->capacity)
        return NULL;
    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_np);

    BSTNode *new_node_p = pool_p->free_head_np;
    __auto_type __chunk_p = (_BSTNodeFreeChunk*)new_node_p;
    pool_p->free_head_np = __chunk_p->next_free_np;
    ++pool_p->count;

    return new_node_p;
}

static BSTNode *_bstpool_new_node(
    _BSTNodePool   *pool_p,
    void           *data 
) {
    BSTNode *node_p = _bstpool_alloc_node(pool_p);
    if (!node_p) return NULL;
    node_p->data        = data;
    node_p->parent_np   = NULL;
    node_p->left_np     = NULL;
    node_p->right_np    = NULL;
    return node_p;
}

static ptrdiff_t _bstpool_get_node_pos(_BSTNodePool *pool_p, BSTNode *node_p) {
    assert(pool_p);
    assert(pool_p->node_arr);
    assert(pool_p->capacity);
    if (
        !pool_p->count || !node_p ||
        node_p < pool_p->node_arr ||
        node_p >= pool_p->node_arr + pool_p->capacity
    ) return -1;
    assert(pool_p->count <= pool_p->capacity);
    assert(pool_p->free_head_np);

    return node_p - pool_p->node_arr;
}


static int _bstpool_free_node(_BSTNodePool *pool_p, BSTNode *node_p) {
    _dbg_log_msg("begin");
    assert(pool_p);
    assert(pool_p->node_arr);
    assert(pool_p->capacity);
    if (
        !pool_p->count || !node_p ||
        node_p < pool_p->node_arr ||
        node_p >= pool_p->node_arr + pool_p->capacity
    ) return -1;
    assert(pool_p->count <= pool_p->capacity);

    memset(node_p, 0, sizeof(*node_p));  
    __auto_type __chunk_p = (_BSTNodeFreeChunk*)node_p;
    __chunk_p->next_free_np = pool_p->free_head_np; 
    pool_p->free_head_np = node_p;
    --pool_p->count;

    _dbg_log_msg("end");
    return 0;
}

bintras_bst *bintras_create_bst(size_t capacity, int (*cmp)(void*, void*)) {
    _dbg_log_msg("begin");
    if (!cmp) goto failed_ret;
    if (!capacity) capacity = BST_CAPACITY_DEFAULT;
    __auto_type bst_p = (BST*)malloc(sizeof(BST));
    if (!bst_p) goto failed_ret;
    memset(bst_p, 0, sizeof(*bst_p));

    bst_p->header_np = _bstpool_init(&bst_p->node_pool, capacity);
    if (!bst_p->header_np) goto failed;
    bst_p->header_np->data      = NULL;
    bst_p->header_np->parent_np = NULL; // root
    bst_p->header_np->left_np   = NULL; // leftmost; min; must be null after bst.is_modified
    bst_p->header_np->right_np  = NULL; // leftmost; max; must be null after bst.is_modified

    bst_p->state_arr = (uint8_t*)malloc(capacity * sizeof(uint8_t));
    if (!bst_p->state_arr) goto failed;
    memset(bst_p->state_arr, BST_NODE_EMPTY, capacity * sizeof(uint8_t));

    bst_p->dead_count   = 0;
    bst_p->cmp_func     = cmp;

    _dbg_log_msg("end");
    return bst_p;
failed:
    bintras_destroy_bst(bst_p);
failed_ret:
    return NULL;
}

void bintras_destroy_bst(bintras_bst *bst_p) {
    _dbg_log_msg("begin");
    if (!bst_p) return;
    _bstpool_fini(&bst_p->node_pool);
    if (bst_p->state_arr) free(bst_p->state_arr);
    free(bst_p);
    _dbg_log_msg("end");
}

// pointer issue
// when a node is not yet allocated
// the first 8 bytes (void *BSTNode::data) of the nodes store
// the ptr to the next node in the pool alloc
// instead of data;
// sizeof(_BSTNodeFreeChunk) == sizeof(BSTNode *_BSTNodeFreeChunk::next_free_np)
//  == 8 == sizeof(void *BSTNode::data)
// This means when a BSTNode is accessed when its still unallocated
// (usually this is a bug)
// void *BSTNode::data is actually the allocator medatata
// (_BSTNodeFreeChunk, and BSTNode *_BSTNodeFreeChunk::next_free_np).
// Assessing BSTNode::data would either break the comparitor (eg: resulting in segfault)
// or causing pool allocator curruption (which is horrible)
// So an easy fix is to check the BST::state_arr at idx of the node in the pool
// to see if the state is BST_NODE_EMPTY
// (which means the node is either freed or has never been allocated)
// and if so skip it or do whatever
// and this MUST be done before comparison or any modification
// A more reliable solution is to separate free list metadata from node array
// but it implies allocation of a dedicated free list and rewriting part of node pool

static const bintras_bst_node *
_bintras_bst_find_insert_pos(
    bintras_bst    *bst_p,
    void           *data,
    int            *out_target_exists_p
) {
    _dbg_log_msg("#0");
    if (!bst_p || !data || !out_target_exists_p) goto failed_ret;
    BSTNode *cur_np     = bst_p->header_np->parent_np;
    BSTNode *parent_np  = NULL;

    *out_target_exists_p = 0; // 0 == false
    while (cur_np) {
        __auto_type nidx = _bstpool_get_node_pos(&bst_p->node_pool, cur_np);
        assert(bst_p->state_arr[nidx] != BST_NODE_EMPTY);
        _dbg_print(
            "(self,parent,left,right)=(%td,%td,%td,%td)",
            nidx,
            _bstpool_get_node_pos(&bst_p->node_pool, cur_np->parent_np),
            (cur_np) ? _bstpool_get_node_pos(&bst_p->node_pool, cur_np->left_np) : -1L,
            (cur_np) ? _bstpool_get_node_pos(&bst_p->node_pool, cur_np->right_np) : -1L
        );
        parent_np = cur_np;
        int cmp_ret = bst_p->cmp_func(data, cur_np->data);
        if (cmp_ret < 0)
            cur_np = cur_np->left_np;
        else if (cmp_ret > 0)
            cur_np = cur_np->right_np;
        else {
            *out_target_exists_p = 1; // i == true
            break;
        };
    }
    _dbg_log_msg("0.ret found\n");
    return parent_np;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL;
}

const bintras_bst_node *
bintras_bst_search(bintras_bst *bst_p, void *data) {
    _dbg_log_msg("#0");
    if (!bst_p || !data) goto failed_ret;
    int target_exists = 0;
    __auto_type np = _bintras_bst_find_insert_pos(bst_p, data, &target_exists);
    return (target_exists) ? np : NULL;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL;
}

const bintras_bst_node *
bintras_bst_insert(bintras_bst *bst_p, void *data) {
    _dbg_log_msg("#0");
    if (!bst_p || !data) return NULL;

    _dbg_log_msg("#1");
    BSTNode *parent_np  = NULL;

    // invariant: node.left < node.self < node.right
    // find the parent
    _dbg_log_msg("#2 search pos");
/*
    if (bst_p->root_np) {
        BSTNode *cur_np = bst_p->root_np;
        while (cur_np) {
            parent_np = cur_np;
            int cmp_ret = bst_p->cmp_func(data, cur_np->data);
            if (cmp_ret < 0)
                cur_np = cur_np->left_np;
            else if (cmp_ret > 0)
                cur_np = cur_np->right_np;
            else
                return cur_np;
        }
        assert(!cur_np);
        assert(parent_np);
    }
*/
    int target_exists = 0;
    parent_np = (BSTNode*)_bintras_bst_find_insert_pos(bst_p, data, &target_exists);
    if (target_exists) return parent_np;

    _dbg_log_msg("#3 new BSTNode()");
    __auto_type node_p = _bstpool_new_node(&bst_p->node_pool, data);
    if (!node_p) return NULL;

    _dbg_log_msg("#4 _bstpool_get_node_pos");
    __auto_type node_pos = _bstpool_get_node_pos(&bst_p->node_pool, node_p);
    bst_p->state_arr[node_pos] = BST_NODE_ALIVE;

    __auto_type parent_nidx = _bstpool_get_node_pos(&bst_p->node_pool, parent_np);
    assert(bst_p->state_arr[parent_nidx] != BST_NODE_EMPTY);

    _dbg_log_msg("#5 check if !root");
    if (!bst_p->header_np->parent_np) {
        bst_p->header_np->parent_np = node_p;
        bst_p->header_np->left_np   = node_p;
        bst_p->header_np->right_np  = node_p;
        _dbg_log_msg("0.ret@no_root\n");
        return node_p;
    }

    // insert the node into parent.left or parent.right
    _dbg_log_msg("#6 insert");
    int cmp_ret = bst_p->cmp_func(data, parent_np->data);
    if (cmp_ret < 0) {
        parent_np->left_np  = node_p;
        if (bst_p->header_np->left_np == parent_np)
            bst_p->header_np->left_np = node_p;
    } else {
        parent_np->right_np = node_p;
        if (bst_p->header_np->right_np == parent_np)
            bst_p->header_np->right_np = node_p;
    }
    node_p->parent_np = parent_np;

    _dbg_log_msg("0.ret\n");
    return node_p;
}

int bintras_bst_mark_dead(bintras_bst *bst_p, bintras_bst_node *node_p) {
    if (!bst_p || !node_p) return -1;
    __auto_type pos = _bstpool_get_node_pos(&bst_p->node_pool, node_p);
    if (pos < 0) return -1;
    bst_p->state_arr[pos] = BST_NODE_DEAD;
    ++bst_p->dead_count;
    return 0;
}

int bintras_bst_node_is_valid(bintras_bst *bst_p, const bintras_bst_node *node_p) {
    if (!bst_p || !node_p) return 0;
    __auto_type pos = _bstpool_get_node_pos(&bst_p->node_pool, (BSTNode*)node_p);
    return (bst_p->state_arr[pos] == BST_NODE_ALIVE);
}

static bintras_bst_node *
_bintras_bst_next_node(
    bintras_bst        *bst_p,
    bintras_bst_node   *node_p
) {
    (void)bst_p;
    BSTNode *cur_np = node_p;
    // case (node.right != null)
    // goto right, then goto leftmost
    if (node_p->right_np) {
        cur_np = cur_np->right_np;
        while (cur_np->left_np) {
            cur_np = cur_np->left_np;
        }
        return cur_np;
    }

    // keep going up (node = node.parent)
    // until turning point (node != node.parent.right)
    while (
        cur_np->parent_np &&
        cur_np == cur_np->parent_np->right_np
    ) {
        cur_np = cur_np->parent_np;
    }
    cur_np = cur_np->parent_np;

    return cur_np;
}

static bintras_bst_node *
_bintras_bst_prev_node(
    bintras_bst        *bst_p,
    bintras_bst_node   *node_p
) {
    (void)bst_p;
    BSTNode *cur_np = node_p;

    // case (node.left != null)
    // goto left, then goto rightmost
    if (node_p->left_np) {
        cur_np = cur_np->left_np;
        while (cur_np->right_np) {
            cur_np = cur_np->right_np;
        }
        return cur_np;
    }

    // keep going up (node = node.parent)
    // until turning point (node != node.parent.left)
    while (
        cur_np->parent_np &&
        cur_np == cur_np->parent_np->left_np
    ) {
        cur_np = cur_np->parent_np;
    }
    cur_np = cur_np->parent_np;

    return cur_np;
}

#define _bintras_bst_leftmost(bst_p) (bst_p)->header_np->left_np
#define _bintras_bst_rightmost(bst_p) (bst_p)->header_np->right_np

const bintras_bst_node *
bintras_bst_iterator_next(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
) {
    if (!bst_p || !node_p) return NULL;
    __auto_type np = _bintras_bst_next_node((bintras_bst*)bst_p, (bintras_bst_node*)node_p);
    return (np) ? np : bst_p->header_np;
}
const bintras_bst_node *
bintras_bst_iterator_prev(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
) {
    if (!bst_p || !node_p) return NULL;
    __auto_type np = _bintras_bst_prev_node((bintras_bst*)bst_p, (bintras_bst_node*)node_p);
    return (np) ? np : bst_p->header_np;
}
const bintras_bst_node *bintras_bst_iterator_begin(bintras_bst *bst_p) {
    if (!bst_p) return NULL;
    return _bintras_bst_leftmost(bst_p);
}
const bintras_bst_node *bintras_bst_iterator_rbegin(bintras_bst *bst_p) {
    if (!bst_p) return NULL;
    return _bintras_bst_rightmost(bst_p);
}
const bintras_bst_node *bintras_bst_iterator_end(bintras_bst *bst_p) {
    (void)bst_p;
    return bst_p->header_np;
}
const bintras_bst_node *bintras_bst_iterator_rend(bintras_bst *bst_p) {
    (void)bst_p;
    return bst_p->header_np;
}

static bintras_bst_node *
_bintras_build_bst_from_nodearr_recursive(
    bintras_bst        *bst_p,
    bintras_bst_node  **node_ptr_arr_begin,
    bintras_bst_node  **node_ptr_arr_end,
    bintras_bst_node   *parent_np
) {
    _dbg_log_msg("_bintras_build_bst_from_nodearr");
    assert(bst_p);
    if (node_ptr_arr_begin >= node_ptr_arr_end) return NULL;

    _dbg_print("gap: %td\n", node_ptr_arr_end - node_ptr_arr_begin);
    __auto_type node_ptr_arr_pivot = node_ptr_arr_begin +
        (node_ptr_arr_end - node_ptr_arr_begin) / 2;

    bintras_bst_node *root_np = *node_ptr_arr_pivot;
    root_np->parent_np = parent_np;
    root_np->left_np =
        _bintras_build_bst_from_nodearr_recursive(
            bst_p,
            node_ptr_arr_begin,
            node_ptr_arr_pivot,
            root_np
        );
    root_np->right_np =
        _bintras_build_bst_from_nodearr_recursive(
            bst_p,
            node_ptr_arr_pivot + 1,
            node_ptr_arr_end,
            root_np
        );
    
    return root_np;
}

int bintras_bst_rebuild(bintras_bst *bst_p) {
    _dbg_log_msg("#0");
    if (!bst_p) return -1;

    // This part is to check the integrity; to be removed
    if (!bst_p->dead_count) {
        _dbg_print("nodec %zu", bst_p->node_pool.count);
        size_t __ctr = 0ul;
        for (
        __auto_type
            it = bst_p->header_np->left_np;
            it != NULL;
            it = _bintras_bst_next_node(bst_p, it)
        ) {
            _dbg_print("it: %zu", __ctr++);
            ptrdiff_t nidx = _bstpool_get_node_pos(&bst_p->node_pool, it);
            assert(bst_p->state_arr[nidx] == BST_NODE_ALIVE);
        }
    }

    _dbg_log_msg("#1 BSTNode **live_np_arr = new BSTNode*[node_pool.count];");
    size_t node_dead_cnt = bst_p->dead_count;
    size_t node_live_cnt = bst_p->node_pool.count - node_dead_cnt;
    __auto_type live_np_arr =
        (BSTNode **)malloc(bst_p->node_pool.count * sizeof(BSTNode *));
    if (!live_np_arr) goto failed_ret;

    _dbg_log_msg("#2 dead_np_arr");
    __auto_type dead_np_arr = live_np_arr + node_live_cnt;
    size_t live_np_arr_pos = 0;
    size_t dead_np_arr_pos = 0;

    _dbg_print("og_deadc: %zu; og_livec: %zu;", node_dead_cnt, node_live_cnt);
    _dbg_log_msg("#3 loop");
    size_t __ctr = 0ul;
    for (
    __auto_type
        it = _bintras_bst_leftmost(bst_p);
        it != NULL;
        it = _bintras_bst_next_node(bst_p, it)
    ) {
        _dbg_print("it: %zu", __ctr++);
        ptrdiff_t nidx = _bstpool_get_node_pos(&bst_p->node_pool, it);
        if (bst_p->state_arr[nidx] == BST_NODE_DEAD) {
            _dbg_print("is_dead: %zu", dead_np_arr_pos);
            dead_np_arr[dead_np_arr_pos++] = it;
            continue;
        }
        _dbg_print("is_alive: %zu", live_np_arr_pos);
        assert(bst_p->state_arr[nidx] == BST_NODE_ALIVE);
        live_np_arr[live_np_arr_pos++] = it;
    }
    _dbg_print("deadc: %zu; livec: %zu;", dead_np_arr_pos, live_np_arr_pos);
    assert(live_np_arr_pos == node_live_cnt);
    assert(dead_np_arr_pos == node_dead_cnt);

    for (size_t i = 0ul; i < node_live_cnt; ++i) {
        __auto_type nidx = _bstpool_get_node_pos(&bst_p->node_pool, live_np_arr[i]);
        assert(bst_p->state_arr[nidx] == BST_NODE_ALIVE);
    }

    _dbg_log_msg("#4 delete[] dead_np_arr;");
    for (size_t i = 0ul; i < node_dead_cnt; ++i) {
        __auto_type nidx = _bstpool_get_node_pos(&bst_p->node_pool, dead_np_arr[i]);
        assert(bst_p->state_arr[nidx] == BST_NODE_DEAD);
        bst_p->state_arr[nidx] = BST_NODE_EMPTY;
        int rc = _bstpool_free_node(&bst_p->node_pool, dead_np_arr[i]);
        assert(rc != -1);
    }
    assert(bst_p->node_pool.count == node_live_cnt);

    dead_np_arr_pos     = 0;
    node_dead_cnt       = 0;
    bst_p->dead_count   = 0;

    _dbg_log_msg("#5 destroy node links");
    for (size_t i = 0ul; i < node_live_cnt; ++i) {
        live_np_arr[i]->parent_np   = NULL;
        live_np_arr[i]->left_np     = NULL;
        live_np_arr[i]->right_np    = NULL;
    }
    dead_np_arr_pos = 0;
    assert(bst_p->node_pool.count == node_live_cnt);

    _dbg_log_msg("#6 rebuild bst recursive");
    BSTNode *root_np =
        _bintras_build_bst_from_nodearr_recursive(
            bst_p,
            live_np_arr,
            live_np_arr + live_np_arr_pos,
            NULL
        );
    assert(root_np);
    bst_p->header_np->parent_np = root_np;

    _dbg_log_msg("0.ret\n");
    return 0;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return -1;
}

const bintras_bst_node *
bintras_bst_lower_bound(bintras_bst *bst_p, void *data) {
    if (!bst_p || !data) goto failed_ret;
    int target_exists = 0;
    __auto_type np = (BSTNode*)_bintras_bst_find_insert_pos(bst_p, data, &target_exists);
    if (!np || np == bst_p->header_np) goto failed_ret;
    return
        (bst_p->cmp_func(data, np->data) > 0)
        ? _bintras_bst_next_node(bst_p, np)
        : np;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL;
}

const bintras_bst_node *
bintras_bst_upper_bound(bintras_bst *bst_p, void *data) {
    if (!bst_p || !data) goto failed_ret;
    int target_exists = 0;
    __auto_type np = (BSTNode*)_bintras_bst_find_insert_pos(bst_p, data, &target_exists);
    if (!np || np == bst_p->header_np) goto failed_ret;
    return
        (bst_p->cmp_func(data, np->data) <= 0)
        ? _bintras_bst_next_node(bst_p, np)
        : np;
failed_ret:
    _dbg_log_msg("-1.ret\n");
    return NULL;
}

