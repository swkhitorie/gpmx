#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdbool.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== Atomic Type Definitions ========== */

typedef struct { volatile int val; }      atomic_int_t;
typedef struct { volatile uint32_t val; }  atomic_uint32_t;
typedef struct { volatile bool val; }     atomic_bool_t;

/* ========== Initialization ========== */

static inline void atomic_int_init(atomic_int_t *a, int val)     { a->val = val; }
static inline void atomic_uint32_init(atomic_uint32_t *a, uint32_t val) { a->val = val; }
static inline void atomic_bool_init(atomic_bool_t *a, bool val) { a->val = val; }

/* ========== Atomic Operations ========== */

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && !defined(__clang__))
#define ATOMIC_USE_CRITICAL 1
#else
#define ATOMIC_USE_CRITICAL 0
#endif

/*
 * If your compiler does not support __atomic builtins,
 * define ATOMIC_USE_CRITICAL and the implementation will fall back to
 * taskENTER_CRITICAL() / taskEXIT_CRITICAL().
 * Note: The critical section fallback is NOT safe to use from ISRs.
 */
#ifdef ATOMIC_USE_CRITICAL

/* Fallback using FreeRTOS critical sections (task context only) */
#define ATOMIC_LOAD(ptr) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_STORE(ptr, val) do { \
    taskENTER_CRITICAL(); \
    (ptr)->val = (val); \
    taskEXIT_CRITICAL(); \
} while (0)

#define ATOMIC_FETCH_ADD(ptr, num) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    (ptr)->val += (num); \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_FETCH_SUB(ptr, num) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    (ptr)->val -= (num); \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_FETCH_AND(ptr, num) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    (ptr)->val &= (num); \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_FETCH_OR(ptr, num) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    (ptr)->val |= (num); \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_FETCH_XOR(ptr, num) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    (ptr)->val ^= (num); \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_FETCH_NAND(ptr, num) ({ \
    typeof((ptr)->val + 0) __ret; \
    taskENTER_CRITICAL(); \
    __ret = (ptr)->val; \
    (ptr)->val = ~((ptr)->val & (num)); \
    taskEXIT_CRITICAL(); \
    __ret; })

#define ATOMIC_CMPEXCH(ptr, expected, desired) ({ \
    bool __ret; \
    taskENTER_CRITICAL(); \
    if ((ptr)->val == *(expected)) { \
        (ptr)->val = (desired); \
        __ret = true; \
    } else { \
        *(expected) = (ptr)->val; \
        __ret = false; \
    } \
    taskEXIT_CRITICAL(); \
    __ret; })

#else /* !ATOMIC_USE_CRITICAL */

/* Use GCC __atomic builtins (lock-free, ISR-safe) */
#define ATOMIC_LOAD(ptr)             __atomic_load_n(&(ptr)->val, __ATOMIC_SEQ_CST)
#define ATOMIC_STORE(ptr, val)       __atomic_store(&(ptr)->val, &(val), __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_ADD(ptr, num)   __atomic_fetch_add(&(ptr)->val, (num), __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_SUB(ptr, num)   __atomic_fetch_sub(&(ptr)->val, (num), __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_AND(ptr, num)   __atomic_fetch_and(&(ptr)->val, (num), __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_OR(ptr, num)    __atomic_fetch_or(&(ptr)->val, (num), __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_XOR(ptr, num)   __atomic_fetch_xor(&(ptr)->val, (num), __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_NAND(ptr, num)  __atomic_fetch_nand(&(ptr)->val, (num), __ATOMIC_SEQ_CST)
#define ATOMIC_CMPEXCH(ptr, expected, desired) \
    __atomic_compare_exchange(&(ptr)->val, (expected), &(desired), false, \
                            __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#endif /* ATOMIC_USE_CRITICAL */

/* ========== atomic_int Operations ========== */

static inline int atomic_int_load(const atomic_int_t *a) {
    return ATOMIC_LOAD(a);
}

static inline void atomic_int_store(atomic_int_t *a, int val) {
    ATOMIC_STORE(a, val);
}

static inline int atomic_int_fetch_add(atomic_int_t *a, int num) {
    return ATOMIC_FETCH_ADD(a, num);
}

static inline int atomic_int_fetch_sub(atomic_int_t *a, int num) {
    return ATOMIC_FETCH_SUB(a, num);
}

static inline int atomic_int_fetch_and(atomic_int_t *a, int num) {
    return ATOMIC_FETCH_AND(a, num);
}

static inline int atomic_int_fetch_or(atomic_int_t *a, int num) {
    return ATOMIC_FETCH_OR(a, num);
}

static inline int atomic_int_fetch_xor(atomic_int_t *a, int num) {
    return ATOMIC_FETCH_XOR(a, num);
}

static inline int atomic_int_fetch_nand(atomic_int_t *a, int num) {
    return ATOMIC_FETCH_NAND(a, num);
}

static inline bool atomic_int_compare_exchange(atomic_int_t *a, int *expected, int desired) {
    return ATOMIC_CMPEXCH(a, expected, desired);
}

/* ========== atomic_uint32_t Operations ========== */

static inline uint32_t atomic_uint32_load(const atomic_uint32_t *a) {
    return ATOMIC_LOAD(a);
}

static inline void atomic_uint32_store(atomic_uint32_t *a, int32_t val) {
    ATOMIC_STORE(a, val);
}

static inline uint32_t atomic_uint32_fetch_add(atomic_uint32_t *a, int32_t num) {
    return ATOMIC_FETCH_ADD(a, num);
}

static inline uint32_t atomic_uint32_fetch_sub(atomic_uint32_t *a, int32_t num) {
    return ATOMIC_FETCH_SUB(a, num);
}

static inline uint32_t atomic_uint32_fetch_and(atomic_uint32_t *a, int32_t num) {
    return ATOMIC_FETCH_AND(a, num);
}

static inline uint32_t atomic_uint32_fetch_or(atomic_uint32_t *a, int32_t num) {
    return ATOMIC_FETCH_OR(a, num);
}

static inline uint32_t atomic_uint32_fetch_xor(atomic_uint32_t *a, int32_t num) {
    return ATOMIC_FETCH_XOR(a, num);
}

static inline uint32_t atomic_uint32_fetch_nand(atomic_uint32_t *a, int32_t num) {
    return ATOMIC_FETCH_NAND(a, num);
}

static inline bool atomic_uint32_compare_exchange(atomic_uint32_t *a, uint32_t *expected, uint32_t desired) {
    return ATOMIC_CMPEXCH(a, expected, desired);
}

/* ========== atomic_bool Operations ========== */

static inline bool atomic_bool_load(const atomic_bool_t *a) {
    return ATOMIC_LOAD(a);
}

static inline void atomic_bool_store(atomic_bool_t *a, bool val) {
    ATOMIC_STORE(a, val);
}

static inline bool atomic_bool_compare_exchange(atomic_bool_t *a, bool *expected, bool desired) {
    return ATOMIC_CMPEXCH(a, expected, desired);
}

/* Arithmetic operations are omitted for bool as they are not meaningful. */

#ifdef __cplusplus
}
#endif

#endif /* ATOMIC_H */