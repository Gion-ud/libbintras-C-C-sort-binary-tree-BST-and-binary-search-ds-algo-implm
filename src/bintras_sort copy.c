// compile with: -std=c23
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "dbg_print.h"
#include <bintras_sort.h>

// namespace: bintras_

typedef unsigned char byte_t;


typedef struct untyped_array_range {
    void       *buf_begin;
    void       *buf_end;
    size_t      elem_size;
} untyped_array_range;

typedef untyped_array_range _arr_range;

[[maybe_unused]]
static inline int untyped_array_range_init(
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

[[maybe_unused]]
static inline void *_arr_range_iter_prev(
    _arr_range     *arr_range_p,
    void           *ptr
) {
    return (byte_t*)ptr - arr_range_p->elem_size;
}
[[maybe_unused]]
static inline void *_arr_range_iter_next(
    _arr_range     *arr_range_p,
    void           *ptr
) {
    return (byte_t*)ptr + arr_range_p->elem_size;
}
[[maybe_unused]]
static inline void _arr_range_iter_inc(
    _arr_range     *arr_range_p,
    void          **ptr_p
) {
    *ptr_p = (byte_t*)*ptr_p + arr_range_p->elem_size;
}
[[maybe_unused]]
static inline void _arr_range_iter_dec(
    _arr_range     *arr_range_p,
    void          **ptr_p
) {
    *ptr_p = (byte_t*)*ptr_p - arr_range_p->elem_size;
}
[[maybe_unused]]
static inline ptrdiff_t _arr_range_iter_diff(
    _arr_range     *arr_range_p,
    void           *ptr1,
    void           *ptr2
) {
    return
        ((byte_t *)ptr1 - (byte_t *)ptr2) /
        (ptrdiff_t)arr_range_p->elem_size;
}

static inline void _bintras_swap(
    void   *elem1_p,
    void   *elem2_p,
    void   *tmp_elem_p,
    size_t  elem_size
) {
    memcpy(tmp_elem_p, elem1_p, elem_size);
    memcpy(elem1_p, elem2_p, elem_size);
    memcpy(elem2_p, tmp_elem_p, elem_size);
}

[[maybe_unused]]
static void *_bintras_lomuto_partition(
    _arr_range     *arr_range_p,
    int           (*cmp)(void *, void *)
) {
    auto __begin    = (byte_t *)arr_range_p->buf_begin;
    auto __end      = (byte_t *)arr_range_p->buf_end;
    auto __pivot_p  = (byte_t *)_arr_range_iter_prev(arr_range_p, __end);
    auto pivot_pos  = __begin;

    unsigned char tmp_elem_buf[arr_range_p->elem_size];
    for (
        auto it = __begin;
        it != _arr_range_iter_prev(arr_range_p, __end);
        _arr_range_iter_inc(arr_range_p, (void**)&it)
    ) {
        if (cmp(it, __pivot_p) < 0) {
            _bintras_swap(pivot_pos, it, tmp_elem_buf, arr_range_p->elem_size);
            _arr_range_iter_inc(arr_range_p, (void**)&pivot_pos);
        }
    }

    _bintras_swap(
        pivot_pos,
        _arr_range_iter_prev(arr_range_p, __end),
        tmp_elem_buf,
        arr_range_p->elem_size
    );
    return pivot_pos;
}

/*
[[maybe_unused]]
static void *_bintras_hoare_partition(
    _arr_range     *arr_range_p,
    int           (*cmp)(void *, void *)
) {
    auto __begin    = (byte_t *)arr_range_p->buf_begin;
    auto __end      = (byte_t *)arr_range_p->buf_end;
    auto _tmp_pivot_p  = __begin + (__end - __begin) / 2;

    unsigned char pivot_data[arr_range_p->elem_size];
    memcpy(pivot_data, _tmp_pivot_p, arr_range_p->elem_size);

    auto left_p     = __begin;
    auto right_p    = _arr_range_iter_prev(arr_range_p, __end);

    while (1) {
        while (cmp(left_p, pivot_data) < 0)
            _arr_range_iter_inc(arr_range_p, (void**)&left_p);

        while (cmp(right_p, pivot_data) > 0)
            _arr_range_iter_dec(arr_range_p, (void**)&right_p);

        if (cmp(left_p, right_p) >= 0)
            return right_p;

        unsigned char tmp_elem_buf[arr_range_p->elem_size];
        _bintras_swap(left_p, right_p, tmp_elem_buf, arr_range_p->elem_size);

        _arr_range_iter_inc(arr_range_p, (void**)&left_p);
        _arr_range_iter_dec(arr_range_p, (void**)&right_p);
    };
}
*/

#define _bintras_partition _bintras_lomuto_partition

typedef struct _arr_range_stack {
    _arr_range *frame_arr;
    size_t      size;
    size_t      capacity;
} _arr_range_stack;

static inline int _arr_range_stack_init(
    _arr_range_stack   *stack_p,
    size_t              capacity,
    _arr_range         *stack_frame_arr
) {
    assert(stack_p && capacity && stack_frame_arr);
    stack_p->frame_arr  = stack_frame_arr;
    stack_p->size       = 0ul;
    stack_p->capacity   = capacity;
    return 0;
}

static inline int _arr_range_stack_push(
    _arr_range_stack   *stack_p,
    _arr_range         *frame_p
) {
    assert(stack_p && frame_p);
    size_t idx = stack_p->size;
    if (idx >= stack_p->capacity) return -1;
    stack_p->frame_arr[idx].buf_begin   = frame_p->buf_begin;
    stack_p->frame_arr[idx].buf_end     = frame_p->buf_end;
    stack_p->frame_arr[idx].elem_size   = frame_p->elem_size;
    ++stack_p->size;
    return 0;
}

static inline int _arr_range_stack_pop(
    _arr_range_stack   *stack_p
) {
    assert(stack_p);
    if (!stack_p->size) return -1;
    --stack_p->size;
    return 0;
}

static inline _arr_range *
_arr_range_stack_top(
    _arr_range_stack   *stack_p
) {
    assert(stack_p);
    return
        (!stack_p->size) ? NULL :
        &stack_p->frame_arr[stack_p->size - 1];
}

static int _bintras_quicksort(
    untyped_array_range    *arr_range_p,
    int                   (*cmp)(void *, void *)
) {
    _dbg_log_msg("#0");
    auto arr_len = _arr_range_iter_diff(
        arr_range_p,
        arr_range_p->buf_end,
        arr_range_p->buf_begin
    ) ;
    if (arr_len <= 1) goto failed_ret;
    assert(arr_len > 0);

    _dbg_log_msg("#1");
    _dbg_print("arr_len:%td", arr_len);
    auto frame_arr = (_arr_range *)malloc(arr_len * sizeof(_arr_range));
    if (!frame_arr) goto failed_ret;
    memset(frame_arr, 0, arr_len * sizeof(*frame_arr));

    _arr_range_stack stack = {0};
    _arr_range_stack_init(&stack, arr_len, frame_arr);

    _dbg_log_msg("#2");
    _arr_range_stack_push(&stack, arr_range_p);

    _dbg_log_msg("#3 loop");
    size_t __it_ctr = 0ul;
    while (stack.size) {
        _dbg_print("stack_size: %zu; __ctr: %zu", stack.size, __it_ctr++);
        auto top_frame_p = _arr_range_stack_top(&stack);
        assert(top_frame_p);
        _arr_range top_frame = {
            .buf_begin  = top_frame_p->buf_begin,
            .buf_end    = top_frame_p->buf_end,
            .elem_size  = top_frame_p->elem_size,
        };
        _arr_range_stack_pop(&stack);

        if (
            _arr_range_iter_diff(
                &top_frame,
                top_frame.buf_end,
                top_frame.buf_begin
            ) <= 1
        ) continue;
        _dbg_log_msg("#3a loop");

        auto pivot_p = _bintras_partition(&top_frame, cmp);
        _dbg_log_msg("#3b loop");

        _arr_range lrange = {
            .buf_begin  = top_frame.buf_begin,
            .buf_end    = pivot_p,
            .elem_size  = top_frame.elem_size
        };
        _arr_range rrange = {
            .buf_begin  = _arr_range_iter_next(&top_frame, pivot_p),
            .buf_end    = top_frame.buf_end,
            .elem_size  = top_frame.elem_size
        };
        _arr_range_stack_push(&stack, &lrange);
        _arr_range_stack_push(&stack, &rrange);
    }
    _dbg_log_msg("#5");
    assert(!stack.size);
    free(frame_arr);

    return 0;
failed_ret:
    _dbg_log_msg("!failed");
    return -1;
}

int bintras_quicksort(
    void   *_arr_begin,
    void   *_arr_end,
    size_t  elem_size,
    int   (*cmp)(void *, void *)
) {
    _dbg_log_msg("");
    if (
        !_arr_begin || !_arr_end
        || !elem_size || !cmp
    ) goto failed_ret;
    _dbg_log_msg("");
    _arr_range arr_view = {
        .buf_begin  = _arr_begin,
        .buf_end    = _arr_end,
        .elem_size  = elem_size,
    };
    auto diff = _arr_range_iter_diff(&arr_view, arr_view.buf_end, arr_view.buf_begin);
    _dbg_print("arr_len: %td", diff);
    _dbg_log_msg("");
    return _bintras_quicksort(&arr_view, cmp);
failed_ret:
    _dbg_log_msg("failed");
    return -1;
}