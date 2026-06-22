#pragma once

#include <stddef.h>


extern void _insertion_sort_array(
    void   *arr_buf,
    size_t  elem_size,
    size_t  arr_len,
    int     (*cmp)(void*, void*)
);

extern void _insertion_sort_argsort(
    void       *arr_buf,
    size_t      elem_size,
    size_t      arr_len,
    ptrdiff_t   idx_arr[],
    int (*cmp)(void*, void*)
);

extern ptrdiff_t _binary_search_array(
    void       *arr_buf,
    size_t      elem_size,
    size_t      arr_len,
    const void *key_p,
    int (*cmp)(void*, void*)
);

extern ptrdiff_t _binary_search_idxarr(
    void       *arr_buf,
    size_t      elem_size,
    size_t      arr_len,
    ptrdiff_t   idx_arr[],
    const void *key_p,
    int         (*cmp)(void*, void*)
);
