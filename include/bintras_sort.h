#pragma once
#include <stddef.h>

extern int bintras_quicksort(
    void   *_arr_begin,
    void   *_arr_end,
    size_t  elem_size,
    int   (*cmp)(void *, void *)
);