#ifndef ATOMIC_BITSET_H
#define ATOMIC_BITSET_H

#include "atomic.h"

#define DEFINE_ATOMIC_BITSET(name, bits) \
    enum { name##_ELEMS = ((bits) + 31) / 32 }; \
    typedef struct { \
        atomic_uint32_t data[name##_ELEMS]; \
    } name; \
    \
    static inline size_t name##_array_index(size_t pos) { return pos / 32; } \
    static inline uint32_t name##_element_mask(size_t pos) { return 1u << (pos % 32); } \
    \
    static inline void name##_init(name *set) { \
        for (size_t i = 0; i < name##_ELEMS; ++i) \
            atomic_uint32_store(&set->data[i], 0); \
    } \
    static inline bool name##_test(const name *set, size_t pos) { \
        size_t idx = name##_array_index(pos); \
        uint32_t mask = name##_element_mask(pos); \
        return (atomic_uint32_load(&set->data[idx]) & mask) != 0; \
    } \
    static inline void name##_set(name *set, size_t pos, bool val) { \
        size_t idx = name##_array_index(pos); \
        uint32_t mask = name##_element_mask(pos); \
        if (val) \
            atomic_uint32_fetch_or(&set->data[idx], mask); \
        else \
            atomic_uint32_fetch_and(&set->data[idx], ~mask); \
    } \
    static inline size_t name##_count(const name *set) { \
        size_t total = 0; \
        for (size_t i = 0; i < name##_ELEMS; ++i) { \
            uint32_t v = atomic_uint32_load(&set->data[i]); \
            while (v) { total += v & 1; v >>= 1; } \
        } \
        return total; \
    } \
    static inline size_t name##_size(void) { return (bits); }


#endif
