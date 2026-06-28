/*
 * bintras_sort.c
 * 
 * Copyright (c) 2026 John Wood
 * All rights reserved.
 * 
 * This file contains the implementation of introsort
 * (made with insertion sort, quicksort and heapsort)
 * for my project libbintras
 * namespace: bintras_
 * 
 *
 */

// compile with: -std=c23
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "dbg_print.h"
#include <bintras_sort.h>
#include <stdalign.h>

typedef unsigned char byte_t;
#define INLINED static inline __attribute__((unused))
#define INTRNL static __attribute__((unused))


typedef struct untyped_array_range {
    void       *buf_begin;
    void       *buf_end;
    size_t      elem_size;
} untyped_array_range;

INLINED int untyped_array_range_init(
    untyped_array_range    *arr_range_p,
    void                   *arr_buf,
    size_t                  arr_len,
    size_t                  elem_size
) {
    if (!arr_range_p || !arr_buf || !elem_size)
        return -1;

    arr_range_p->buf_begin  = arr_buf;
    arr_range_p->buf_end    = (byte_t*)arr_buf + arr_len * elem_size;
    arr_range_p->elem_size  = elem_size;
    return 0;
}

INLINED void *_ptr_iter_prev(void *ptr, size_t elem_size) {
    return (byte_t*)ptr - elem_size;
}
INLINED void *_ptr_iter_next(void *ptr, size_t elem_size) {
    return (byte_t*)ptr + elem_size;
}
INLINED void _ptr_iter_inc(byte_t **elem_pp, size_t elem_size) {
    *elem_pp = *elem_pp + elem_size;
}
INLINED void _ptr_iter_dec(byte_t **elem_pp, size_t elem_size) {
    *elem_pp = *elem_pp - elem_size;
}
INLINED ptrdiff_t _ptr_iter_diff(
    void   *ptr1,
    void   *ptr2,
    size_t  elem_size
) {
    return
        ((byte_t*)ptr1 - (byte_t*)ptr2) / (ptrdiff_t)elem_size;
}
INLINED void *_ptr_iter_add(
    void       *base,
    ptrdiff_t   count,
    size_t      elem_size
) {
    return
        (byte_t*)base + count * elem_size;
}
INLINED void *_ptr_iter_sub(
    void       *base,
    ptrdiff_t   count,
    size_t      elem_size
) {
    return
        (byte_t*)base - count * elem_size;
}
INLINED void *_ptr_iter_step(
    void       *base,
    ptrdiff_t   idx,
    size_t      elem_size
) {
    return
        (byte_t*)base + idx * elem_size;
}
#define _ptr_iter_at(base, idx, elem_size) _ptr_iter_step(base, idx, elem_size)

INLINED void _bintras_memswap(
    void   *elem1_p,
    void   *elem2_p,
    size_t  elem_size
) {
    __auto_type __e1p = (byte_t*)elem1_p;
    __auto_type __e2p = (byte_t*)elem2_p;
    byte_t tmp = 0u;
    while (elem_size--) {
        tmp     = *__e1p;
        *__e1p  = *__e2p;
        *__e2p  = tmp;
        ++__e1p; ++__e2p;
    }
}

INTRNL void _bintras_insertion_sort(
    byte_t     *_arr_begin,             /* arr.begin() */
    byte_t     *_arr_end,               /* arr.end() */
    byte_t     *_key_buf,               /* &key; user_provided mem */
    size_t      elem_size,              /* sizeof(arr[0]) */
    int       (*cmp)(void *, void *)    /* cmp(&elem1, &elem2) callback */
) {
    ptrdiff_t arr_len = _ptr_iter_diff(_arr_end, _arr_begin, elem_size);
    for (ptrdiff_t i = 1; i < arr_len; ++i) {
        memcpy(
            _key_buf,
            _ptr_iter_add(_arr_begin, i, elem_size),
            elem_size
        );
        ptrdiff_t j = i;
        while (
            j > 0 &&
            cmp(
                _ptr_iter_add(_arr_begin, j - 1, elem_size),
                _key_buf
            ) > 0
        ) {
            memcpy(
                _ptr_iter_add(_arr_begin, j, elem_size),
                _ptr_iter_add(_arr_begin, j - 1, elem_size),
                elem_size
            );
            --j;
        }
        memcpy(
            _ptr_iter_add(_arr_begin, j, elem_size),
            _key_buf,
            elem_size
        );
    }
}


INLINED void *_bintras_lomuto_partition(
    byte_t     *_arr_begin,             /* arr.begin() */
    byte_t     *_arr_end,               /* arr.end() */
    byte_t     *_pivot_buf,             /* &out_pivot; user_provided mem */
    size_t      elem_size,              /* sizeof(arr[0]) */
    int       (*cmp)(void *, void *)    /* cmp(&elem1, &elem2) callback */
) {
    memcpy(
        _pivot_buf,
        _ptr_iter_prev(_arr_end, elem_size),
        elem_size
    );
    /* pivot = *(arr.end() - 1) */
    __auto_type _pivot_pos_p = _arr_begin;  /* pivot_pos_p = arr.begin() */

    for (
        __auto_type it = _arr_begin;                /* it = arr.begin() */
        it != _ptr_iter_prev(_arr_end, elem_size);  /* it != arr.end() */
        _ptr_iter_inc(&it, elem_size)               /* ++it */
    ) {
        if (cmp(it, _pivot_buf) < 0) {
            _bintras_memswap(_pivot_pos_p, it, elem_size);  /* swap(&out_pivot, it) */
            _ptr_iter_inc(&_pivot_pos_p, elem_size);        /* ++pivot_pos_p */
        }
    }

    _bintras_memswap(
        _pivot_pos_p,
        _ptr_iter_prev(_arr_end, elem_size),
        elem_size
    );  /* swap(pivot_pos_p, arr.end() - 1) */
    return (_pivot_pos_p == _arr_begin)
        ? _ptr_iter_next(_pivot_pos_p, elem_size)
        : _pivot_pos_p;
    /*
        The checking: we don't want to return
        the same element if there are duplicates because
        the partition must return a mid that works with
        range iter such that [begin, mid) and [mid, end)
        works fine
    */
}

INLINED void *_bintras_hoare_partition(
    byte_t     *_arr_begin,             /* arr.begin() */
    byte_t     *_arr_end,               /* arr.end() */
    byte_t     *_pivot_buf,             /* &out_pivot; user_provided mem */
    size_t      elem_size,              /* sizeof(arr[0]) */
    int       (*cmp)(void *, void *)    /* cmp(&elem1, &elem2) callback */
) {
    _dbg_log_msg("#0");

    /* arr.length() = arr.end() - arr.begin() */
    __auto_type _arr_len =
        _ptr_iter_diff(_arr_end, _arr_begin, elem_size);

    /* arr.pivot() = top_range.begin() + top_range.length() / 2) */
    memcpy(
        _pivot_buf,
        _ptr_iter_add(_arr_begin, _arr_len / 2, elem_size),
        elem_size
    );

    __auto_type lptr = _arr_begin;
    __auto_type rptr = (byte_t*)_ptr_iter_prev(_arr_end, elem_size);

    while (1) {
        _dbg_log_msg("loop");
        while (cmp(lptr, _pivot_buf) < 0)
            _ptr_iter_inc(&lptr, elem_size);

        while (cmp(rptr, _pivot_buf) > 0)
            _ptr_iter_dec(&rptr, elem_size);

        if (lptr >= rptr)
            return lptr;

        _bintras_memswap(lptr, rptr, elem_size);

        _ptr_iter_inc(&lptr, elem_size);
        _ptr_iter_dec(&rptr, elem_size);
    }
}

#define _bintras_partition _bintras_lomuto_partition

typedef struct _raw_range {
    void       *_begin;
    void       *_end;
} _raw_range;

typedef struct _range_stack {
    _raw_range *range_arr;
    size_t      size;
    size_t      capacity;
} _range_stack;

INLINED int _range_stack_push(
    _range_stack   *stack_p,
    _raw_range     *range_p
) {
    size_t idx = stack_p->size;
    if (idx >= stack_p->capacity) return -1;
    stack_p->range_arr[idx]._begin  = range_p->_begin;
    stack_p->range_arr[idx]._end    = range_p->_end;
    ++stack_p->size;
    return 0;
}
INLINED int _range_stack_pop(_range_stack *stack_p) {
    if (!stack_p->size) return -1;
    --stack_p->size;
    return 0;
}
INLINED _raw_range *_range_stack_top(_range_stack *stack_p) {
    return
        (!stack_p->size) ? NULL :
        &stack_p->range_arr[stack_p->size - 1];
}
INLINED void _range_stack_reset(_range_stack *stack_p) {
    stack_p->size = 0ul;
}


#define _RANGE_STACK_DEPTH_MAX      64u
#define _INSERTION_SORT_THRESHOLD   16u  
#define _ELEM_SIZE_MAX              64u

static int _bintras_quicksort(
    byte_t     *_arr_begin,             /* arr.begin() */
    byte_t     *_arr_end,               /* arr.end() */
    size_t      elem_size,              /* sizeof(arr[0]) */
    int       (*cmp)(void *, void *)    /* cmp(&elem1, &elem2) callback */
) {
    _dbg_log_msg("#0");
    ptrdiff_t arr_len = _ptr_iter_diff(_arr_end, _arr_begin, elem_size);
    if (arr_len <= 1) goto failed_ret;
    assert(arr_len > 0);

    alignas(max_align_t)
    byte_t _tmp_elem_buf[_ELEM_SIZE_MAX];

    _dbg_log_msg("#1");
    if (arr_len <= _INSERTION_SORT_THRESHOLD) {
        _bintras_insertion_sort(
            _arr_begin,
            _arr_end,
            _tmp_elem_buf,
            elem_size,
            cmp
        );
        return 0;
    }

    _dbg_log_msg("#1");
    _dbg_print("arr_len:%td", arr_len);
    _raw_range range_arr[_RANGE_STACK_DEPTH_MAX];
    memset(range_arr, 0, _RANGE_STACK_DEPTH_MAX * sizeof(*range_arr));

    _range_stack stack = {
        .range_arr  = range_arr,
        .size       = 0ul,
        .capacity   = _RANGE_STACK_DEPTH_MAX,
    };

    _dbg_log_msg("#2");
    _raw_range top_range = { _arr_begin, _arr_end };
    _range_stack_push(&stack, &top_range);    

    _dbg_log_msg("#3 loop");
    //size_t __it_ctr = 0ul;
    while (stack.size) {
        //_dbg_print("stack_size: %zu; __ctr: %zu", stack.size, __it_ctr++);
        _raw_range *top_range_p = _range_stack_top(&stack);   /* stack.top() */
        assert(top_range_p);
        top_range._begin   = top_range_p->_begin;
        top_range._end     = top_range_p->_end;
        _range_stack_pop(&stack); /* stack.pop() */

        ptrdiff_t top_range_len =
            _ptr_iter_diff(top_range._end, top_range._begin, elem_size);
        _dbg_print("range_len: %td", top_range_len);
        if (top_range_len <= 1) continue;
        if (top_range_len <= _INSERTION_SORT_THRESHOLD) {
            _dbg_log_msg("using insertion sort");
            _bintras_insertion_sort(
                top_range._begin,
                top_range._end,
                _tmp_elem_buf,
                elem_size,
                cmp
            );
            continue;
        }

        _dbg_log_msg("using quicksort");
        /* top_range.length() = top_range.end() - top_range.begin() */
        void *_split = _bintras_partition(
            top_range._begin,
            top_range._end,
            _tmp_elem_buf,
            elem_size,
            cmp
        );

        _raw_range lrange = { top_range._begin, _split };
        _raw_range rrange = { _split, top_range._end };

        if (stack.size + 2 > stack.capacity) {
            // switch to heapsort; to be implemented
            goto failed_ret;
        }
        if (
            _ptr_iter_diff(lrange._end, lrange._begin, elem_size) < 
            _ptr_iter_diff(rrange._end, rrange._begin, elem_size)
        ) {
            _range_stack_push(&stack, &lrange);
            _range_stack_push(&stack, &rrange);
        } else {
            _range_stack_push(&stack, &rrange);
            _range_stack_push(&stack, &lrange);
        }
    }
    _dbg_log_msg("#5");
    assert(!stack.size);

    return 0;
failed_ret:
    _dbg_log_msg("!failed");
    return -1;
}

// public
int bintras_quicksort(
    void   *_arr_begin,
    void   *_arr_end,
    size_t  elem_size,
    int   (*cmp)(void *, void *)
) {
    _dbg_log_msg("");
    if (
        !_arr_begin || !_arr_end
        || !elem_size || elem_size > _ELEM_SIZE_MAX || !cmp
    ) goto failed_ret;
    _dbg_log_msg("");

    return _bintras_quicksort(_arr_begin, _arr_end, elem_size, cmp);
failed_ret:
    _dbg_log_msg("failed");
    return -1;
}