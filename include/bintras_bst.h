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
extern int bintras_bst_mark_dead(bintras_bst *bst_p, void *data);
extern const bintras_bst_node *bintras_bst_insert(bintras_bst *bst_p, void *data);
extern const bintras_bst_node *bintras_bst_search(bintras_bst *bst_p, void *data);
extern const bintras_bst_node *
bintras_bst_next_node(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
);
extern const bintras_bst_node *
bintras_bst_prev_node(
    const bintras_bst      *bst_p,
    const bintras_bst_node *node_p
);
extern const bintras_bst_node *bintras_min_node(bintras_bst *bst_p);
extern const bintras_bst_node *bintras_max_node(bintras_bst *bst_p);