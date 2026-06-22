#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <skvmap.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unwind.h>


#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

typedef struct _cstr_kv {
    const char *key;
    const char *value;
} cstr_kv_t;

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

extern void *_Znwy(size_t);
extern void _ZdlPv(void*);

#define operator_new(size) _Znwy(size)
#define operator_delete(size) _ZdlPv(size)

int main() {
    size_t len = get_array_type_length(kvtbl);

    skvmap *kvm_p = create_skvmap();
    assert(kvm_p);

    puts("put kv");
    for (size_t i = 0ul; i < len; ++i) {
        kvmap_entry e = {
            (char*)kvtbl[i].key,
            strlen(kvtbl[i].key),
            (char*)kvtbl[i].value
        };
        int rc = skvmap_insert(kvm_p, &e);
        assert(rc != -1);
    }
    puts("");

    puts("get all kv");
    for (size_t i = 0ul; i < len; ++i) {
        __auto_type data = (char*)skvmap_get(kvm_p, (char*)kvtbl[i].key);
        assert(data);
        printf("%s -> %s\n", kvtbl[i].key, data);
    }
    puts("");

    puts("get all kv 2");
    skvmap_iterator it;
    skvmap_iterator it_begin = skvmap_iterator_begin(kvm_p);
    skvmap_iterator it_end = skvmap_iterator_end(kvm_p);

    for (
        it = it_begin;
        it != it_end;
        it = skvmap_iterator_next(kvm_p, it)
    ) {
        kvmap_entry *ep = skvmap_iterator_deref(kvm_p, it);
        printf("%s -> %s\n", ep->key_buf, (char*)ep->data);
    }
    puts("");

    skvmap_erase(kvm_p, "close");
    skvmap_erase(kvm_p, "lseek");
    skvmap_erase(kvm_p, "std::vector");
    skvmap_erase(kvm_p, "munmap");

    for (
        it = it_begin;
        it != it_end;
        it = skvmap_iterator_next(kvm_p, it)
    ) {
        kvmap_entry *ep = skvmap_iterator_deref(kvm_p, it);
        if (!ep) continue;
        printf("%s -> %s\n", ep->key_buf, (char*)ep->data);
    }
    puts("");

    puts("cxx: new");
    char *buf = (char*)operator_new(64);
    memset(buf, 0, 64);
    char *tmp = "libstdc++:new;libstdc++:delete;\n";
    memcpy(buf, tmp, 32);
    printf("%s", buf);
    operator_delete(buf);



    puts("cxx: new");
    buf = (char*)malloc(64);
    memset(buf, 0, 64);
    tmp = "libstdc++:new;libstdc++:delete;\n";
    memcpy(buf, tmp, 32);
    printf("%s", buf);
    operator_delete(buf);

    puts("cxx: new");
    buf = (char*)operator_new(64);
    memset(buf, 0, 64);
    tmp = "libstdc++:new;libstdc++:delete;\n";
    memcpy(buf, tmp, 32);
    printf("%s", buf);
    free(buf);

    puts("cxx: new");
    buf = (char*)operator_new(UINT64_MAX);
    memset(buf, 0, 64);
    tmp = "libstdc++:new;libstdc++:delete;\n";
    memcpy(buf, tmp, 32);
    printf("%s", buf);
    operator_delete(buf);


    destroy_skvmap(kvm_p);
    return 0;
}