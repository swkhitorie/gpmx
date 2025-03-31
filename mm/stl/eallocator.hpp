
#ifndef __EALLOCATOR_H_
#define __EALLOCATOR_H_

#include "edefault_alloc_template.hpp"

namespace ESAF
{

template <class _Tp>
class eallocator
{
  protected:
    typedef ealloc _Alloc;

  public:
    typedef _Tp value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef size_t size_type;

  public:
    pointer allocate(size_type __n, const void * = 0)
    {
        return __n != 0 ? static_cast<_Tp *>(_Alloc::allocate(__n * sizeof(_Tp))) : 0;
    }
    void deallocate(pointer __p, size_type __n)
    {
        _Alloc::deallocate(__p, __n * sizeof(value_type));
    }
};

}  // namespace ESAF

#endif  // eallocator_H
