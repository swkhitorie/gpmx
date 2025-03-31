
#ifndef __EDEFAULT_ALLOC_TEMPLATE_H_
#define __EDEFAULT_ALLOC_TEMPLATE_H_

#include <stddef.h>
#include "emalloc_alloc_template.hpp"

namespace ESAF
{

enum
{
    __ALIGN = 8
};
enum
{
    __MAX_BYTES = 128
};
enum
{
    __NFREELISTS = __MAX_BYTES / __ALIGN
};

/*!
 * @brief Secondary allocator, copied from STL
 */
template <bool trheads, int inst>
class __edefault_alloc_template
{
  private:
    static size_t RoundUp(size_t bytes) { return (((bytes) + __ALIGN - 1) & ~(__ALIGN - 1)); }

  private:
    union obj
    {
        union obj *free_list_link;
    };

  private:
    static obj *volatile free_list[__NFREELISTS];
    static size_t FREELIST_INDEX(size_t bytes) { return (((bytes) + __ALIGN - 1) / __ALIGN - 1); }
    static void *refill(size_t size);
    static char *chunk_alloc(size_t size, int &nobjs);

    static char *start_free;
    static char *end_free;
    static size_t heap_size;

  public:
    static void *allocate(size_t n);
    static void deallocate(void *p, size_t n);
};

template <bool threads, int inst>
char *__edefault_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char *__edefault_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
size_t __edefault_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
typename __edefault_alloc_template<threads, inst>::obj
    *volatile __edefault_alloc_template<threads, inst>::free_list[__NFREELISTS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

template <bool threads, int inst>
void *__edefault_alloc_template<threads, inst>::allocate(size_t n)
{
    obj *volatile *my_free_list;
    obj *result;

    if (n > (size_t)__MAX_BYTES)
    {
        return malloc_alloc::allocate(n);
    }

    my_free_list = free_list + FREELIST_INDEX(n);
    result       = *my_free_list;
    if (result == 0)
    {
        void *r = refill(RoundUp(n));
        return r;
    }
    *my_free_list = result->free_list_link;
    return (result);
}

template <bool threads, int inst>
void __edefault_alloc_template<threads, inst>::deallocate(void *p, size_t n)
{
    obj *q = (obj *)p;
    obj *volatile *my_free_list;

    if (n > (size_t)__MAX_BYTES)
    {
        malloc_alloc::deallocate(p, n);
        return;
    }
    my_free_list      = free_list + FREELIST_INDEX(n);
    q->free_list_link = *my_free_list;
    *my_free_list     = q;
}

template <bool threads, int inst>
char *__edefault_alloc_template<threads, inst>::chunk_alloc(size_t size, int &nobjs)
{
    char *result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left  = end_free - start_free;

    if (bytes_left >= total_bytes)
    {
        result = start_free;
        start_free += total_bytes;
        return (result);
    }
    else if (bytes_left >= size)
    {
        nobjs       = bytes_left / size;
        total_bytes = size * nobjs;
        result      = start_free;
        start_free += total_bytes;
        return (result);
    }
    else
    {
        size_t bytes_to_get = 2 * total_bytes + RoundUp(heap_size >> 4);
        if (bytes_left > 0)
        {
            obj *volatile *my_free_list         = free_list + FREELIST_INDEX(bytes_left);
            ((obj *)start_free)->free_list_link = *my_free_list;
            *my_free_list                       = (obj *)start_free;
        }
        // get memory from first allocator (system pool)
        start_free = (char *)malloc_alloc::allocate_no_handler(bytes_to_get);
        if (0 == start_free)
        {
            int i;
            obj *volatile *my_free_list, *p;
            for (i = size; i <= __MAX_BYTES; i += __ALIGN)
            {
                my_free_list = free_list + FREELIST_INDEX(i);
                p            = *my_free_list;
                if (0 != p)
                {
                    *my_free_list = p->free_list_link;
                    start_free    = (char *)p;
                    end_free      = start_free + i;
                    return (chunk_alloc(size, nobjs));
                }
            }
            end_free = 0;
            // get memory from first allocator - oom_handler
            start_free = (char *)malloc_alloc::allocate(bytes_to_get);
        }
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return (chunk_alloc(size, nobjs));
    }
}

template <bool threads, int inst>
void *__edefault_alloc_template<threads, inst>::refill(size_t n)
{
    int nobjs   = 20;
    char *chunk = chunk_alloc(n, nobjs);
    obj *volatile *my_free_list;
    obj *result;
    obj *current_obj;
    obj *next_obj;
    int i;
    if (1 == nobjs)
        return (chunk);
    my_free_list = free_list + FREELIST_INDEX(n);

    result        = (obj *)chunk;
    *my_free_list = next_obj = (obj *)(chunk + n);
    for (i = 1;; ++i)
    {
        current_obj = next_obj;
        next_obj    = (obj *)((char *)next_obj + n);
        if (nobjs - 1 == i)
        {
            current_obj->free_list_link = 0;
            break;
        }
        else
        {
            current_obj->free_list_link = next_obj;
        }
    }
    return (result);
}
typedef __edefault_alloc_template<false, 0> ealloc;

}  // namespace ESAF

#endif  // edefault_alloc_template_H
