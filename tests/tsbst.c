#include <assert.h>
#include <bintras_bst.h>
#include <stdio.h>
#include <vector.h>
#include <stddef.h>
#include <stdio.h>

int cmp_int(void *p1, void *p2) {
    return *(int*)p1 - *(int*)p2;
}

#define get_array_type_length(arr) sizeof(arr) / sizeof(*arr)

int main() {
    puts("-- decl arr --");
    int arr[] = {
        1024, 32, 72, 64, 727, 6, 7, 69, 67, 31,
        13, 11, 17, 23, 47, 3, 5, 2, 111, 255,
        1024, 512, 1080, 1080, 24, 48, 36, 36, 67, 67, 11
    };
    __auto_type arr_len = get_array_type_length(arr);

    puts("-- print arr unsorted --");
    for (__auto_type i = 0ul; i < arr_len; ++i) {
        printf("%d\n", arr[i]);
    }
    puts("\n");

    puts("-- bintras_create_bst --");
    __auto_type bst_p = bintras_create_bst(32, &cmp_int);

    puts("-- insert all keys --");
    for (__auto_type i = 0ul; i < arr_len; ++i) {
        __auto_type rc = bintras_bst_insert(bst_p, &arr[i]);
        assert(rc);
    }

    puts("-- forward iteration --");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }

    puts("-- backwards iteration --");
    for (
    __auto_type
        it = bintras_max_node(bst_p);
        it != NULL;
        it = bintras_bst_prev_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }

    puts("-- get all keys --");
    for (__auto_type i = 0ul; i < arr_len; ++i) {
        __auto_type rc = bintras_bst_search(bst_p, &arr[i]);
        assert(rc);
    }

    puts("-- rebuild --");
    bintras_bst_rebuild(bst_p);

    puts("-- forward iteration --");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }

    puts("-- backwards iteration --");
    for (
    __auto_type
        it = bintras_max_node(bst_p);
        it != NULL;
        it = bintras_bst_prev_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }


    puts("-- range query --");
    int l = 0;
    int r = 101;
    for (
    __auto_type
        it = bintras_bst_lower_bound(bst_p, &l);
        it != bintras_bst_lower_bound(bst_p, &r);
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }

    puts("-- delete some entries --");
    l = 14;
    r = 67;
    for (
    __auto_type
        it = bintras_bst_lower_bound(bst_p, &l);
        it != bintras_bst_upper_bound(bst_p, &r);
        it = bintras_bst_next_node(bst_p, it)
    ) {
        bintras_bst_mark_dead(bst_p, (bintras_bst_node*)it);
    }

    puts("-- forward iteration --");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        assert(it);
        if (!bintras_bst_node_is_valid(bst_p, it))
            continue;
        printf("%d\n", *(int*)it->data);
    }

    puts("-- rebuild --");
    bintras_bst_rebuild(bst_p);

    puts("-- forward iteration --");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        printf("%d\n", *(int*)it->data);
    }
    puts("-- backwards iteration --");
    for (
    __auto_type
        it = bintras_max_node(bst_p);
        it != NULL;
        it = bintras_bst_prev_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }


    puts("-- insert all keys --");
    for (__auto_type i = 0ul; i < arr_len; ++i) {
        __auto_type rc = bintras_bst_insert(bst_p, &arr[i]);
        assert(rc);
    }
    bintras_bst_rebuild(bst_p);


    puts("-- forward iteration --");
    for (
    __auto_type
        it = bintras_min_node(bst_p);
        it != NULL;
        it = bintras_bst_next_node(bst_p, it)
    ) {
        printf("%d\n", *(int*)it->data);
    }
    puts("-- backwards iteration --");
    for (
    __auto_type
        it = bintras_max_node(bst_p);
        it != NULL;
        it = bintras_bst_prev_node(bst_p, it)
    ) {
        assert(it);
        printf("%d\n", *(int*)it->data);
    }


    puts("-- bintras_destroy_bst --");
    bintras_destroy_bst(bst_p);
    return 0;
}