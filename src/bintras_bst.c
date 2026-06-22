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
    BSTNode        *root_np;        // [2]; non owning ref
    size_t          dead_count;     // [5]; dead node count
    int (*cmp_nodes)(void*, void*);
};
typedef bintras_bst BST;

#define BST_CAPACITY_DEFAULT 32u
#define BST_NODE_EMPTY  0u
#define BST_NODE_ALIVE  1u
#define BST_NODE_DEAD   2u

static int _bstpool_init(_BSTNodePool *pool_p, size_t capacity) {
    _dbg_log_msg("begin");
    assert(pool_p);
    assert(capacity);
    memset(pool_p, 0, sizeof(*pool_p));

    pool_p->node_arr = (BSTNode*)malloc(capacity * sizeof(BSTNode));
    if (!pool_p->node_arr) return -1;

    for (size_t i = 0ul; i < capacity - 1; ++i) {
        __auto_type __chunk_p = (_BSTNodeFreeChunk*)&pool_p->node_arr[i];
        __chunk_p->next_free_np = &pool_p->node_arr[i + 1];
    }
    __auto_type __chunk_p = (_BSTNodeFreeChunk*)&pool_p->node_arr[capacity - 1];
    __chunk_p->next_free_np = NULL;

    pool_p->free_head_np    = &pool_p->node_arr[0];
    pool_p->count           = 0;
    pool_p->capacity        = capacity;

    _dbg_log_msg("end");
    return 0;
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
    _dbg_log_msg("begin");
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

    _dbg_log_msg("end");
    return new_node_p;
}

static BSTNode *_bstpool_new_node(
    _BSTNodePool   *pool_p,
    void           *data 
) {
    _dbg_log_msg("begin");
    BSTNode *node_p = _bstpool_alloc_node(pool_p);
    if (!node_p) return NULL;
    node_p->data        = data;
    node_p->parent_np   = NULL;
    node_p->left_np     = NULL;
    node_p->right_np    = NULL;
    return node_p;
    _dbg_log_msg("end");
}

static ptrdiff_t _bstpool_get_node_pos(_BSTNodePool *pool_p, BSTNode *node_p) {
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
    assert(pool_p->free_head_np);

    return node_p - pool_p->node_arr;
    _dbg_log_msg("end");
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

    int rc = _bstpool_init(&bst_p->node_pool, capacity);
    if (rc < 0) goto failed;
    bst_p->state_arr = (uint8_t*)malloc(capacity * sizeof(uint8_t));
    if (!bst_p->state_arr) goto failed;
    memset(bst_p->state_arr, 0xFF, capacity * sizeof(uint8_t));

    bst_p->root_np      = NULL;
    bst_p->dead_count   = 0;
    bst_p->cmp_nodes    = cmp;

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

const bintras_bst_node *
bintras_bst_insert(bintras_bst *bst_p, void *data) {
    _dbg_log_msg("begin");
    if (!bst_p || !data) return NULL;
    __auto_type node_p = _bstpool_new_node(&bst_p->node_pool, data);
    if (!node_p) return NULL;

    ptrdiff_t node_pos = _bstpool_get_node_pos(&bst_p->node_pool, node_p);
    bst_p->state_arr[node_pos] = BST_NODE_ALIVE;

    if (!bst_p->root_np) {
        bst_p->root_np = node_p;
        return node_p;
    }

    BSTNode *cur_np     = bst_p->root_np;
    BSTNode *parent_np  = NULL;

    // invariant: node.left < node.self < node.right
    // find the parent
    while (cur_np) {
        parent_np = cur_np;
        int cmp_ret = bst_p->cmp_nodes(data, cur_np->data);
        if (cmp_ret < 0)
            cur_np = cur_np->left_np;
        else if (cmp_ret > 0)
            cur_np = cur_np->right_np;
        else {
            _bstpool_free_node(&bst_p->node_pool, node_p);
            bst_p->state_arr[node_pos] = BST_NODE_EMPTY;
            return cur_np;
        }
    }

    // insert the node into parent.left or parent.right
    int cmp_ret = bst_p->cmp_nodes(data, parent_np->data);
    if (cmp_ret < 0) {
        parent_np->left_np  = node_p;
    } else {
        parent_np->right_np = node_p;
    }

    node_p->parent_np = parent_np;
    _dbg_log_msg("end");
    return node_p;
}

const bintras_bst_node *
bintras_bst_search(bintras_bst *bst_p, void *data) {
    _dbg_log_msg("begin");
    if (!bst_p || !data) return NULL;
    BSTNode *cur_np     = bst_p->root_np;
    BSTNode *parent_np  = NULL;

    while (cur_np) {
        parent_np = cur_np;
        int cmp_ret = bst_p->cmp_nodes(data, cur_np->data);
        if (cmp_ret < 0)
            cur_np = cur_np->left_np;
        else if (cmp_ret > 0)
            cur_np = cur_np->right_np;
        else
            return parent_np;
    }

    _dbg_log_msg("end.failed");
    return NULL;
}

int bintras_bst_mark_dead(bintras_bst *bst_p, void *data) {
    if (!bst_p || !data) return -1;
    BSTNode *target_np = (BSTNode*)bintras_bst_search(bst_p, data);
    if (!target_np) return -1;
    __auto_type pos = _bstpool_get_node_pos(&bst_p->node_pool, target_np);
    if (pos < 0) return -1;
    bst_p->state_arr[pos] = BST_NODE_DEAD;
    return 0;
}

static const bintras_bst_node *
_bintras_bst_leftmost(bintras_bst *bst_p) {
    _dbg_log_msg("begin");
    if (!bst_p) return NULL;
    BSTNode *cur_np = bst_p->root_np;
    while (cur_np->left_np) {
        cur_np = cur_np->left_np;
    }
    _dbg_log_msg("end");
    return cur_np;
}

static const bintras_bst_node *
_bintras_bst_rightmost(bintras_bst *bst_p) {
    _dbg_log_msg("begin");
    if (!bst_p) return NULL;
    BSTNode *cur_np = bst_p->root_np;
    while (cur_np->right_np) {
        cur_np = cur_np->right_np;
    }
    _dbg_log_msg("end");
    return cur_np;
}


static const bintras_bst_node *
_bintras_bst_next_node(
    bintras_bst        *bst_p,
    bintras_bst_node   *node_p
) {
    _dbg_log_msg("begin");
    if (!bst_p || !node_p) return NULL;
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

    _dbg_log_msg("end");
    return cur_np;
}

static const bintras_bst_node *
_bintras_bst_prev_node(
    bintras_bst        *bst_p,
    bintras_bst_node   *node_p
) {
    _dbg_log_msg("begin");
    if (!bst_p || !node_p) return NULL;
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

    _dbg_log_msg("end");
    return cur_np;
}

const bintras_bst_node *
bintras_bst_next_node(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
) {
    return _bintras_bst_next_node((bintras_bst*)bst_p, (bintras_bst_node*)node_p);
}
const bintras_bst_node *
bintras_bst_prev_node(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
) {
    return _bintras_bst_prev_node((bintras_bst*)bst_p, (bintras_bst_node*)node_p);
}
const bintras_bst_node *bintras_min_node(bintras_bst *bst_p) {
    return _bintras_bst_leftmost(bst_p);
}
const bintras_bst_node *bintras_max_node(bintras_bst *bst_p) {
    return _bintras_bst_rightmost(bst_p);
}