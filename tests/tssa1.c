#include "sorted_array.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;


int cmp_cstr_kv(void *kv1_p, void *kv2_p) {
    return strcmp(((cstr_kv_t*)kv1_p)->key, ((cstr_kv_t*)kv2_p)->key);
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

int main() {
    size_t len = get_array_type_length(kvtbl);

    for (size_t i = 0u; i < len; ++i) {
        printf("%s -> %s\n", kvtbl[i].key, kvtbl[i].value);
    }
    puts("");

    _insertion_sort_array(
        kvtbl,
        sizeof(*kvtbl),
        len,
        &cmp_cstr_kv
    );

    for (size_t i = 0ul; i < len; ++i) {
        printf("%s -> %s\n", kvtbl[i].key, kvtbl[i].value);
    }
    puts("");

    for (size_t i = 0ul; i != len; ++i) {
        __auto_type ret = _binary_search_array(
            kvtbl,
            sizeof(*kvtbl),
            len,
            &kvtbl[i],
            &cmp_cstr_kv
        );
        if (ret < 0) {
            puts("failed....");
            continue;
        }
       printf("%s -> %s\n", kvtbl[ret].key, kvtbl[ret].value); 
    }
    puts("");


    cstr_kv_t k = {"mmap", NULL};
    __auto_type ret = _binary_search_array(
        kvtbl,
        sizeof(*kvtbl),
        len,
        &k,
        &cmp_cstr_kv
    );
    if (ret != -1) {
        printf("%s -> %s\n", kvtbl[ret].key, kvtbl[ret].value);
    }


    return 0;
}