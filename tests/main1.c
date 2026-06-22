#include "sorted_array.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>



int cmp_cstr(void *_cstr1_p, void *_cstr2_p) {
    return strcmp(*(char**)_cstr1_p, *(char**)_cstr2_p);
}

int cmp_int(void *i1_p, void *i2_p) {
    return (*(int*)i1_p - *(int*)i2_p);
}

#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

int main() {
    char *cstr_arr[] = {
        "open",
        "close",
        "read",
        "write",
        "lseek",
        "fsync",
        "mmap",
        "munmap",
    };

    size_t cstr_arr_len = get_array_type_length(cstr_arr);
    size_t cstr_idx_arr[cstr_arr_len];

    for (
        __auto_type it = cstr_arr;
        it != cstr_arr + cstr_arr_len;
        ++it
    ) {
        printf("%s\n", *it);
    }
    puts("");

    _insertion_sort_argsort(
        cstr_arr,
        sizeof(char*),
        cstr_arr_len,
        cstr_idx_arr,
        &cmp_cstr
    );

    puts("cstr_idx_arr");
    for (size_t i = 0ul; i != cstr_arr_len; ++i) {
        printf("%zu\n", cstr_idx_arr[i]);
    }
    puts("");
    for (size_t i = 0ul; i != cstr_arr_len; ++i) {
        printf("%s\n", cstr_arr[cstr_idx_arr[i]]);
    }
    puts("");




    return 0;
}