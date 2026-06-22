#include <stddef.h>
#include <stdlib.h>
#include <string.h>


void _insertion_sort_array(
    void   *arr_buf,
    size_t  elem_size,
    size_t  arr_len,
    int     (*cmp)(void*, void*)
) {
    if (!arr_buf || !elem_size || arr_len < 2 || !cmp) return;
    unsigned char *__arr_buf = (unsigned char*)arr_buf;

    for (size_t i = 1; i < arr_len; ++i) {
        unsigned char __key[elem_size];
        memcpy(__key, __arr_buf + i * elem_size, elem_size);
        size_t j = i;
        while (
            j > 0 &&
            cmp(__arr_buf + (j - 1) * elem_size, __key) > 0
        ) {
            memcpy(
                __arr_buf + j * elem_size,
                __arr_buf + (j - 1) * elem_size,
                elem_size
            );
            --j;
        }
        memcpy(__arr_buf + j * elem_size, __key, elem_size);
    }
}


void _insertion_sort_argsort(
    void       *arr_buf,
    size_t      elem_size,
    size_t      arr_len,
    ptrdiff_t   idx_arr[],
    int (*cmp)(void*, void*)
) {
    if (!arr_buf || !elem_size || arr_len < 2 || !cmp || !idx_arr) return;
    unsigned char *__arr_buf = (unsigned char*)arr_buf;

    for (size_t i = 0ul; i < arr_len; ++i) {
        idx_arr[i] = (ptrdiff_t)i;
    }

    for (size_t i = 1; i < arr_len; ++i) {
        int key = idx_arr[i];
        size_t j = i;
        while (
            j > 0 &&
            cmp(
                __arr_buf + (idx_arr[j - 1]) * elem_size,
                __arr_buf + (key) * elem_size
            ) > 0
        ) {
            idx_arr[j] = idx_arr[j - 1];
            --j;
        }
        idx_arr[j] = key;
    }
}

ptrdiff_t _binary_search_idxarr(
    void       *arr_buf,
    size_t      elem_size,
    size_t      arr_len,
    ptrdiff_t   idx_arr[],
    const void *key_p,
    int (*cmp)(void*, void*)
) {
    if (!arr_buf || !elem_size || !arr_len || !key_p) return -1;

    ptrdiff_t begin_idx = 0;
    ptrdiff_t end_idx = (ptrdiff_t)arr_len - 1;

    unsigned char *__arr_buf = (unsigned char*)arr_buf;
    while (begin_idx <= end_idx) {
        ptrdiff_t pivot_idx = begin_idx + (end_idx - begin_idx) / 2;
        int cmp_ret = cmp(__arr_buf + idx_arr[pivot_idx] * elem_size, (void*)key_p);
        if (cmp_ret == 0) {
            return (ptrdiff_t)pivot_idx;
        }
        if (cmp_ret < 0) {
            begin_idx = pivot_idx + 1;
        } else {
            end_idx = pivot_idx - 1;
        }
    }
    return -1;
}

ptrdiff_t _binary_search_array(
    void       *arr_buf,
    size_t      elem_size,
    size_t      arr_len,
    const void *key_p,
    int         (*cmp)(void*, void*)
) {
    if (!arr_buf || !elem_size || !arr_len || !key_p) return -1;

    ptrdiff_t begin_idx = 0;
    ptrdiff_t end_idx = (ptrdiff_t)arr_len - 1;

    unsigned char *__arr_buf = (unsigned char*)arr_buf;
    while (begin_idx <= end_idx) {
        ptrdiff_t pivot_idx = begin_idx + (end_idx - begin_idx) / 2;
        int cmp_ret = cmp(__arr_buf + pivot_idx * elem_size, (void*)key_p);
        if (cmp_ret == 0) {
            return (ptrdiff_t)pivot_idx;
        }
        if (cmp_ret < 0) {
            begin_idx = pivot_idx + 1;
        } else {
            end_idx = pivot_idx - 1;
        }
    }
    return -1;
}
