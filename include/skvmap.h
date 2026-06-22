#pragma once
#include <stddef.h>

typedef struct kvmap_entry {
    char   *key_buf;
    size_t  key_len;
    void   *data;
} kvmap_entry;
typedef struct skvmap skvmap;
extern skvmap *create_skvmap();
extern void destroy_skvmap(skvmap *kvm_p);
extern int skvmap_insert(skvmap *kvm_p, kvmap_entry *ent_p);
extern void *skvmap_get(skvmap *kvm_p, char *key_cstr);
extern int skvmap_erase(skvmap *kvm_p, char *key_cstr);

typedef ptrdiff_t *skvmap_iterator;

extern skvmap_iterator skvmap_iterator_begin(skvmap *kvm_p);
extern skvmap_iterator skvmap_iterator_end(skvmap *kvm_p);
extern skvmap_iterator skvmap_iterator_next(skvmap *kvm_p, skvmap_iterator it);
extern kvmap_entry *skvmap_iterator_deref(skvmap *kvm_p, skvmap_iterator it);
