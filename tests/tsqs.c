#include <bintras_sort.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


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

#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

int main() {
    cstr_kv_t kvtbl[] = {
        {"open", "libc:fcntl.h:open"},
        {"close", "libc:unistd.h:close"},
        {"read", "libc:unistd.h:read"},
        {"write", "libc:unistd.h:write"},
        {"munmap", "libc:sys/mman.h:munmap"},
        {"ftruncate", "libc:unistd.h:ftruncate"},
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
    __auto_type kvtbl_len = get_array_type_length(kvtbl);

    puts("-- print all kv --");
    for (__auto_type i = 0ul; i < kvtbl_len; ++i) {
        printf("%s -> %s\n", kvtbl[i].key, kvtbl[i].value);
    }
    puts("");

    int rc = bintras_quicksort(
        kvtbl,
        kvtbl + kvtbl_len,
        sizeof(*kvtbl),
        cmp_cstr_kv
    );
    assert(rc != -1);

    puts("-- print all kv --");
    for (__auto_type i = 0ul; i < kvtbl_len; ++i) {
        printf("%s -> %s\n", kvtbl[i].key, kvtbl[i].value);
    }
    puts("");


    return 0;
}