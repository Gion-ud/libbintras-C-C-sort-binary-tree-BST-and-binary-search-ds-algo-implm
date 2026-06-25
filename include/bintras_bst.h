#pragma once

#include <stddef.h>
typedef struct bintras_bst bintras_bst;
typedef struct bintras_bst_node bintras_bst_node;
struct bintras_bst_node {
    void               *data;
    bintras_bst_node   *parent_np;
    bintras_bst_node   *left_np;
    bintras_bst_node   *right_np;
};


typedef struct bintras_bst_node BSTNode;


extern bintras_bst *bintras_create_bst(size_t capacity, int (*cmp)(void*, void*));
extern void bintras_destroy_bst(bintras_bst *bst_p);
extern const bintras_bst_node *bintras_bst_search(bintras_bst *bst_p, void *data);
extern int bintras_bst_mark_dead(bintras_bst *bst_p, bintras_bst_node *node_p);
extern int bintras_bst_node_is_valid(bintras_bst *bst_p, const bintras_bst_node *node_p);
extern const bintras_bst_node *bintras_bst_insert(bintras_bst *bst_p, void *data);
extern const bintras_bst_node *bintras_bst_search(bintras_bst *bst_p, void *data);

extern const bintras_bst_node *
bintras_bst_iterator_next(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
);
extern const bintras_bst_node *
bintras_bst_iterator_prev(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
);
extern const bintras_bst_node *bintras_bst_iterator_begin(bintras_bst *bst_p);
extern const bintras_bst_node *bintras_bst_iterator_end(bintras_bst *bst_p);
extern const bintras_bst_node *bintras_bst_iterator_rbegin(bintras_bst *bst_p);
extern const bintras_bst_node *bintras_bst_iterator_rend(bintras_bst *bst_p);

#define bintras_min_node(bst_p) bintras_bst_iterator_begin(bst_p)
#define bintras_max_node(bst_p) bintras_bst_iterator_rbegin(bst_p)

extern const bintras_bst_node *bintras_bst_lower_bound(bintras_bst *bst_p, void *data);
extern const bintras_bst_node *bintras_bst_upper_bound(bintras_bst *bst_p, void *data);
extern int bintras_bst_rebuild(bintras_bst *bst_p);

// debug build
#include <stdint.h>
typedef struct _bintras_bst_implm {
    unsigned char       node_pool[32];  // [0]; ownership
    uint8_t            *state_arr;      // [1]; ownership
    bintras_bst_node   *root_np;        // [2]; non owning ref
    size_t              dead_count;     // [5]; dead node count
    int (*cmp_nodes)(void*, void*);
} _bintras_bst_implm;


