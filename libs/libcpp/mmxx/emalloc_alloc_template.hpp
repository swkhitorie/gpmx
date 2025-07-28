
#ifndef __EMALLOC_ALLOC_TEMPLATE_H_
#define __EMALLOC_ALLOC_TEMPLATE_H_

#include <stddef.h>
#include "libxx.hxx"

#define USE_THROW_CXX 0
#if USE_THROW_CXX
    #if __cplusplus >= 201103L
        #define __THROW_BAD_EALLOC throw
    #endif
#else
    #define __THROW_BAD_EALLOC
#endif

namespace ESAF
{

/*!
 * @brief Primary allocator, copied from STL
 */
template <int inst>
class __emalloc_alloc_template
{
  private:
    static void *oom_malloc(size_t);
    static void *oom_realloc(void *, size_t);
    static void (*__malloc_alloc_oom_handler)();

  public:
    static void *allocate_no_handler(size_t n)
    {
        void *result = sys_mem_malloc(n);
        return result;
    }
    static void *allocate(size_t n)
    {
        void *result = sys_mem_malloc(n);
        if (0 == result)
            result = oom_malloc(n);
        return result;
    }
    static void deallocate(void *p, size_t /* n */) { sys_mem_free(p); }
    static void *reallocate(void *p, size_t /* old_sz */, size_t new_sz)
    {
        void *result = sys_mem_realloc(p, 0, new_sz);
        if (0 == result)
            result = oom_realloc(p, new_sz);
        return result;
    }
    static void (*set_malloc_handler(void (*f)()))()
    {
        void (*old)()              = __malloc_alloc_oom_handler;
        __malloc_alloc_oom_handler = f;
        return (old);
    }
};

template <int inst>
void (*__emalloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void *__emalloc_alloc_template<inst>::oom_malloc(size_t n)
{
    void (*my_malloc_handler)();
    void *result;

    for (;;)
    {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler)
        {
            __THROW_BAD_EALLOC;
        }
        (*my_malloc_handler)();
        result = sys_mem_malloc(n);
        if (result)
            return (result);
    }
}

template <int inst>
void *__emalloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
{
    void (*my_malloc_handler)();
    void *result;

    for (;;)
    {
        my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == my_malloc_handler)
        {
            __THROW_BAD_EALLOC;
        }
        (*my_malloc_handler)();
        result = sys_mem_realloc(p, 0, n);
        if (result)
            return (result);
    }
}

typedef __emalloc_alloc_template<0> malloc_alloc;

template <class T, class Alloc>
class simple_alloc
{
  public:
    static T *allocate(size_t n) { return 0 == n ? 0 : (T *)Alloc::allocate(n * sizeof(T)); }
    static T *allocate(void) { return (T *)Alloc::allocate(sizeof(T)); }
    static void deallocate(T *p, size_t n)
    {
        if (0 != n)
            Alloc::deallocate(p, n * sizeof(T));
    }
    static void deallocate(T *p) { Alloc::deallocate(p, sizeof(T)); }
};

}  // namespace ESAF

#endif  // emalloc_alloc_template_H
