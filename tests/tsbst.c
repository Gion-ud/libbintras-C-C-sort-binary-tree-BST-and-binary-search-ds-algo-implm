#include <assert.h>
#include <bintras_bst.h>
#include <stdio.h>
#include <vector.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;

int cmp_cstr_kv(void *kv1_p, void *kv2_p) {
    __auto_type cstr1 = ((cstr_kv_t*)kv1_p)->key;
    __auto_type cstr2 = ((cstr_kv_t*)kv2_p)->key;
    assert(cstr1 && cstr2);
    return strcmp(cstr1, cstr2);
}

int cmp_int(void *p1, void *p2) {
    return *(int*)p1 - *(int*)p2;
}


cstr_kv_t kvtbl[] = {
    {"open", "libc:fcntl.h:open"},
    {"close", "libc:unistd.h:close"},
    {"read", "libc:unistd.h:read"},
    {"write", "libc:unistd.h:write"},
    {"lseek", "libc:unistd.h:lseek"},
    {"mmap", "libc:sys/mman.h:mmap"},
    {"munmap", "libc:sys/mman.h:munmap"},
    {"ftruncate", "libc:unistd.h:ftruncate"},
    {"fsync", "libc:unistd.h:fsync"},
    {"msync", "libc:sys/mman.h:msync"},
    {"std::vector<int>::push_back", "libstdc++:vector:vector<int>::push_back"},
    {"malloc", "libc:stdlib.h:malloc"},
    {"calloc", "libc:stdlib.h:calloc"},
    {"realloc", "libc:stdlib.h:realloc"},
    {"free", "libc:stdlib.h:free"},
};
#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

int main() {
    puts("bintras_bst *bst_p = new bintras_bst(entc, &cmp);");
    bintras_bst *bst_p = bintras_create_bst(32, &cmp_cstr_kv);
    assert(bst_p);
    size_t kvc = get_array_type_length(kvtbl);

    Vector(bintras_bst_node*) v = vector_create(bintras_bst_node*);

    puts("1. insert all kv");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type rc = bintras_bst_insert(bst_p, &kvtbl[i]);
        assert(rc);
    }
    puts("");

    puts("7. rebuild");
    bintras_bst_rebuild(bst_p);

    puts("2. get all kv");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        cstr_kv_t *ent_p = (cstr_kv_t*)it->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    char *cstr = "mmap_";
    // this is only working because the in mem layout of cstr_kv is:
    // [key_cstr_p][val_cstr_p]
    // and since the cmp ignores [val_cstr_p]
    // we can just take the addr of a cstr_ptr
    // so [key_cstr_p][val_cstr_p] and [key_cstr_p] are identical to cmp
    // but i need to be cautious about this hack
    __auto_type __np = (bintras_bst_node*)bintras_bst_search(bst_p, &cstr);
    assert(!__np);
    cstr = "ftruncate_";
    __np = (bintras_bst_node*)bintras_bst_search(bst_p, &cstr);
    assert(!__np);

    puts("3. get all kv desc");
    for (
    __auto_type
        it = bintras_max_node(bst_p);
        it != NULL;
        it = bintras_bst_prev_node(bst_p, it)
    ) {
        assert(it);
        cstr_kv_t *ent_p = (cstr_kv_t*)it->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    puts("4. search all kv");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type np = (bintras_bst_node*)bintras_bst_search(bst_p, &kvtbl[i]);
        assert(np);
        vector_push_back(&v, &np);
        cstr_kv_t *ent_p = (cstr_kv_t*)np->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    size_t del_idx_arr[] = { 0, 1, 2, 3, 5, 7 };
    __auto_type del_idx_arr_len = get_array_type_length(del_idx_arr);
    puts("5. del some entries");
    for (__auto_type i = 0ul; i < del_idx_arr_len; ++i) {
        bintras_bst_mark_dead(bst_p, v[del_idx_arr[i]]);
    }
    puts("");
    vector_destroy(v);
    assert(((_bintras_bst_implm*)bst_p)->dead_count == del_idx_arr_len);

    puts("6. search all kv");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type np = bintras_bst_search(bst_p, &kvtbl[i]);
        assert(np);
        printf("ENTRY_STATE: %hd\n", ((_bintras_bst_implm*)bst_p)->state_arr[i]);
        if (!bintras_bst_node_is_valid(bst_p, np)) {
            //puts("this entry is DEADBEEF");
        }
        cstr_kv_t *ent_p = (cstr_kv_t*)np->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    puts("7. rebuild");
    bintras_bst_rebuild(bst_p);

    puts("8. get all kv asc");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        cstr_kv_t *ent_p = (cstr_kv_t*)it->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    puts("9. insert all kv (again)");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type rc = bintras_bst_insert(bst_p, &kvtbl[i]);
        assert(rc);
    }
    puts("");
    puts("9.1 rebuild");
    bintras_bst_rebuild(bst_p);


    puts("10. get all kv");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        cstr_kv_t *ent_p = (cstr_kv_t*)it->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    puts("11. search all kv");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type np = bintras_bst_search(bst_p, &kvtbl[i]);
        assert(np);
        if (!bintras_bst_node_is_valid(bst_p, np)) {
            puts("this entry is DEADBEEF");
        }
        cstr_kv_t *ent_p = (cstr_kv_t*)np->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    puts("12. get all kv");
    cstr = "freedom";
    for (
    __auto_type
        it = bintras_bst_lower_bound(bst_p, &cstr);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        cstr_kv_t *ent_p = (cstr_kv_t*)it->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");

    puts("13. get all kv");
    char *lcstr = "mmap";
    char *rcstr = "write";
    for (
    __auto_type
        it = bintras_bst_lower_bound(bst_p, &lcstr);
        it != bintras_bst_upper_bound(bst_p, &rcstr);
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        cstr_kv_t *ent_p = (cstr_kv_t*)it->data;
        printf("%s -> %s\n", ent_p->key, ent_p->value);
    }
    puts("");


    puts("delete bst_p;");
    bintras_destroy_bst(bst_p);
/*
    int arr[] = {6, 7, 69, 67, 31, 13, 11, 17, 23, 47, 3, 5, 2, 111};
    __auto_type arr_len = get_array_type_length(arr);
    bst_p = bintras_create_bst(32, &cmp_int);
    for (__auto_type i = 0ul; i < arr_len; ++i) {
        __auto_type rc = bintras_bst_insert(bst_p, &arr[i]);
        assert(rc);
    }
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        int val = *(int*)it->data;
        printf("%d\n", val);
    }

    bintras_destroy_bst(bst_p);
*/
    return 0;
}