#include "sorted_array.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>


int cmp_cstr(void *_cstr1_p, void *_cstr2_p) {
    return strcmp(*(char**)_cstr1_p, *(char**)_cstr2_p);
}


#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

int main() {

    char *key_arr[] = {
        "open",
        "close",
        "read",
        "write",
        "lseek",
        "mmap",
        "munmap",
    };

    char *val_arr[] = {
        "fcntl.h:open",
        "unistd.h:close",
        "unistd.h:read",
        "unistd.h:write",
        "unistd.h:lseek",
        "sys/mman.h:mmap",
        "sys/mman.h:munmap",
    };

    size_t len = get_array_type_length(key_arr);
    size_t idx_arr[len];

    for (size_t i = 0u; i < len; ++i) {
        printf("%s -> %s\n", key_arr[i], val_arr[i]);
    }
    puts("");

    _insertion_sort_argsort(
        key_arr,
        sizeof(*key_arr),
        len,
        idx_arr,
        &cmp_cstr
    );

    for (size_t i = 0ul; i < len; ++i) {
        printf("%s -> %s\n", key_arr[idx_arr[i]], val_arr[idx_arr[i]]);
    }
    puts("");

    for (size_t i = 0ul; i != len; ++i) {
        __auto_type ret = _binary_search_idxarr(
            key_arr,
            sizeof(*key_arr),
            len,
            idx_arr,
            &key_arr[i],
            &cmp_cstr
        );
        if (ret < 0) {
            puts("failed....");
            continue;
        }
        printf("%s -> %s\n", key_arr[idx_arr[ret]], val_arr[idx_arr[ret]]); 
    }
    puts("");

    char *k = "mmap";
    __auto_type ret = _binary_search_idxarr(
        key_arr,
        sizeof(*key_arr),
        len,
        idx_arr,
        &k,
        &cmp_cstr
    );
    if (ret != -1) {
        printf("%s -> %s\n", key_arr[idx_arr[ret]], val_arr[idx_arr[ret]]); 
    }


    return 0;
}