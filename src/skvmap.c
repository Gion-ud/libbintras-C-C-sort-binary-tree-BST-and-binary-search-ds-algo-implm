#include <skvmap.h>
#include <assert.h>
#include <sorted_array.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vector.h>
#include <stdbool.h>

#define KVMAP_ENT_EMPTY 0
#define KVMAP_ENT_VALID 1
#define KVMAP_ENT_DEAD  2

typedef struct skvmap {
    Vector(kvmap_entry)     ent_vec;
    Vector(unsigned char)   cntl_vec;
    Vector(ptrdiff_t)       idx_vec;
    size_t                  entcnt;
    size_t                  entcap;
    bool                    is_sorted;
} skvmap;


static int _cmp_kvmap_entry(void *ent1_p, void *ent2_p) {
    __auto_type __ent1_p = (kvmap_entry*)ent1_p;
    __auto_type __ent2_p = (kvmap_entry*)ent2_p;
    if (__ent1_p->key_len == __ent2_p->key_len)
        return memcmp(__ent1_p->key_buf, __ent2_p->key_buf, __ent1_p->key_len);
    return strncmp(__ent1_p->key_buf, __ent2_p->key_buf, __ent1_p->key_len);
}

skvmap *create_skvmap() {
    __auto_type kvm_p = (skvmap*)malloc(sizeof(skvmap));
    if (!kvm_p) goto failed_ret;
    memset(kvm_p, 0, sizeof(*kvm_p));
    kvm_p->ent_vec  = vector_create(kvmap_entry);
    kvm_p->cntl_vec = vector_create(unsigned char);
    kvm_p->idx_vec  = vector_create(ptrdiff_t);
    if (!kvm_p->ent_vec || !kvm_p->cntl_vec || !kvm_p->idx_vec)
        goto failed;
    kvm_p->entcap       = 0;
    kvm_p->entcnt       = 0;
    kvm_p->is_sorted    = false;
    return kvm_p;
failed:
    destroy_skvmap(kvm_p);
failed_ret:
    return NULL;
}

int skvmap_insert(skvmap *kvm_p, kvmap_entry *ent_p) {
    if (!kvm_p || !ent_p) goto failed_ret;
    assert(kvm_p->ent_vec);
    assert(kvm_p->cntl_vec);
    assert(kvm_p->idx_vec);

    size_t idx = kvm_p->entcnt;
    int rc = vector_push_back(&kvm_p->ent_vec, ent_p);
    if (rc < 0) goto failed_ret;
    assert((int)idx == rc);

    unsigned char state = KVMAP_ENT_VALID;
    rc = vector_push_back(&kvm_p->cntl_vec, &state);
    if (rc < 0) goto failed;
    assert((int)idx == rc);
    
    rc = vector_push_back(&kvm_p->idx_vec, &idx);
    if (rc < 0) goto failed1;
    assert((int)idx == rc);

    kvm_p->is_sorted = false;
    ++kvm_p->entcnt;
    return 0;
failed1:
    vector_pop_back(kvm_p->cntl_vec);
failed:
    vector_pop_back(kvm_p->ent_vec);
failed_ret:
    return -1;
}


void *skvmap_get(skvmap *kvm_p, char *key_cstr) {
    if (!kvm_p || !key_cstr || !kvm_p->entcnt) goto failed_ret;
    assert(kvm_p->ent_vec);
    assert(kvm_p->cntl_vec);
    assert(kvm_p->idx_vec);

    if (!kvm_p->is_sorted) {
        _insertion_sort_argsort(
            kvm_p->ent_vec,
            sizeof(kvmap_entry),
            vector_size(kvm_p->ent_vec),
            kvm_p->idx_vec,
            &_cmp_kvmap_entry
        );
        kvm_p->is_sorted = true;
    }

    kvmap_entry __ent = { key_cstr, strlen(key_cstr), NULL };
    ptrdiff_t ret = _binary_search_idxarr(
        kvm_p->ent_vec,
        sizeof(kvmap_entry),
        vector_size(kvm_p->ent_vec),
        kvm_p->idx_vec,
        &__ent,
        &_cmp_kvmap_entry
    );
    if (ret < 0L) goto failed_ret;
    if (kvm_p->cntl_vec[kvm_p->idx_vec[ret]] != KVMAP_ENT_VALID)
        goto failed_ret;

    return kvm_p->ent_vec[kvm_p->idx_vec[ret]].data;
failed_ret:
    return NULL;
}

int skvmap_erase(skvmap *kvm_p, char *key_cstr) {
    if (!kvm_p || !key_cstr || !kvm_p->entcnt) goto failed_ret;
    assert(kvm_p->ent_vec);
    assert(kvm_p->cntl_vec);
    assert(kvm_p->idx_vec);

    if (!kvm_p->is_sorted) {
        _insertion_sort_argsort(
            kvm_p->ent_vec,
            sizeof(kvmap_entry),
            vector_size(kvm_p->ent_vec),
            kvm_p->idx_vec,
            &_cmp_kvmap_entry
        );
        kvm_p->is_sorted = true;
    }

    kvmap_entry __ent = { key_cstr, strlen(key_cstr), NULL };
    ptrdiff_t ret = _binary_search_idxarr(
        kvm_p->ent_vec,
        sizeof(kvmap_entry),
        vector_size(kvm_p->ent_vec),
        kvm_p->idx_vec,
        &__ent,
        &_cmp_kvmap_entry
    );
    if (ret < 0L) goto failed_ret;
    kvm_p->cntl_vec[kvm_p->idx_vec[ret]] = KVMAP_ENT_DEAD;

    return 0;
failed_ret:
    return -1;
}

void destroy_skvmap(skvmap *kvm_p) {
    if (!kvm_p) return;
    if (kvm_p->ent_vec)     vector_destroy(kvm_p->ent_vec);
    if (kvm_p->cntl_vec)    vector_destroy(kvm_p->cntl_vec);
    if (kvm_p->idx_vec)     vector_destroy(kvm_p->idx_vec);
    free(kvm_p);
}

int compact_skvmap(skvmap **kvm_pp) {
    if (!kvm_pp || !*kvm_pp) goto failed_ret;

    skvmap *old_kvm_p = *kvm_pp;
    skvmap *new_kvm_p = create_skvmap();
    if (!new_kvm_p) goto failed;


    skvmap_iterator it          = NULL;
    skvmap_iterator it_begin    = skvmap_iterator_begin(old_kvm_p);
    skvmap_iterator it_end      = skvmap_iterator_end(old_kvm_p);

    for (it = it_begin; it != it_end; ++it) {
        __auto_type __ep = skvmap_iterator_deref(old_kvm_p, it);
        if (!__ep) continue;
        int rc = skvmap_insert(new_kvm_p, __ep);
        if (rc < 0) goto failed;
    }



failed:
    destroy_skvmap(new_kvm_p);
failed_ret:
    return -1;
}

skvmap_iterator skvmap_iterator_begin(skvmap *kvm_p) {
    if (!kvm_p->is_sorted) {
        _insertion_sort_argsort(
            kvm_p->ent_vec,
            sizeof(kvmap_entry),
            vector_size(kvm_p->ent_vec),
            kvm_p->idx_vec,
            &_cmp_kvmap_entry
        );
        kvm_p->is_sorted = true;
    }

    return kvm_p->idx_vec;
}

skvmap_iterator skvmap_iterator_end(skvmap *kvm_p) {
    if (!kvm_p->is_sorted) return NULL;
    return (ptrdiff_t*)vector_end(kvm_p->idx_vec) - 1;
}

skvmap_iterator skvmap_iterator_next(skvmap *kvm_p, skvmap_iterator it) {
    (void)kvm_p;
    return it + 1;
}

kvmap_entry *skvmap_iterator_deref(skvmap *kvm_p, skvmap_iterator it) {
    (void)kvm_p;
    if (kvm_p->cntl_vec[*it] != KVMAP_ENT_VALID) return NULL;
    return &kvm_p->ent_vec[*it];
}

