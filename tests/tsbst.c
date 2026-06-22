#include <assert.h>
#include <bintras_bst.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;

int cmp_cstr_kv(void *kv1_p, void *kv2_p) {
    return strcmp(((cstr_kv_t*)kv1_p)->key, ((cstr_kv_t*)kv2_p)->key);
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

    puts("1. insert all kv");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type rc = bintras_bst_insert(bst_p, &kvtbl[i]);
        assert(rc);
    }
    puts("");

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

    puts("2.a get all kv desc");
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

    puts("3. search all kv");
    for (__auto_type i = 0ul; i < kvc; ++i) {
        __auto_type np = bintras_bst_search(bst_p, &kvtbl[i]);
        assert(np);
        cstr_kv_t *ent_p = (cstr_kv_t*)np->data;
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