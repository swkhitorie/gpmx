
#ifndef _EMEMPROCESS_H_
#define _EMEMPROCESS_H_

#include "econstructor.hpp"
#include "etype_traits.hpp"
#include "eiterator_traits.hpp"

namespace ESTL
{

template <class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T &val)
{
    while (first != last)
    {
        *first = val;
        ++first;
    }
}

template <class OutputIterator, class Size, class T>
OutputIterator fill_n(OutputIterator first, Size n, const T &val)
{
    while (n > 0)
    {
        *first = val;
        ++first;
        --n;
    }
    return first;
}

template <class BidirectionalIterator1, class BidirectionalIterator2>
BidirectionalIterator2 copy_backward(BidirectionalIterator1 first,
                                     BidirectionalIterator1 last,
                                     BidirectionalIterator2 result)
{
    while (last != first)
        *(--result) = *(--last);
    return result;
}

template <class InputIterator, class OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
{
    while (first != last)
    {
        *result = *first;
        ++result;
        ++first;
    }
    return result;
}

template <class ForwardIterator, class size, class T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, size n, const T &x, __etrue_type)
{
	// If it is a POD type, 
	// it can be obtained through the independent variable derivation 
	// mechanism of the function template
    return fill_n(first, n, x);
}

template <class ForwardIterator, class size, class T>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, size n, const T &x, __efalse_type)
{

    ForwardIterator cur = first;
    for (; n > 0; --n, ++cur)
    {
        _construct(&*cur, x);
    }
    return cur;
}

template <class InputIterator, class ForwardIterator, class T>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first,
                                                InputIterator last,
                                                ForwardIterator result,
                                                __etrue_type)
{
    return copy(first, last, result);
}

template <class InputIterator, class ForwardIterator, class T>
ForwardIterator __uninitialized_copy_aux(InputIterator first,
                                         InputIterator last,
                                         ForwardIterator result,
                                         __efalse_type)
{
    ForwardIterator cur = first;
    for (; first != last; ++first, ++cur)
    {
        _construct(&*cur, *first);
    }
    return cur;
}

template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first,
                                     ForwardIterator last,
                                     const T &x,
                                     __etrue_type)
{
    fill(first, last, x);
}

template <class ForwardIterator, class T>
void __uninitialized_fill_aux(ForwardIterator first,
                              ForwardIterator last,
                              const T &x,
                              __efalse_type)
{
    ForwardIterator cur = first;
    for (; cur != last; ++cur)
    {
        _construct(&*cur, x);
    }
    return cur;
}

template <class InputIterator, class ForwardIterator, class T>
inline ForwardIterator __uninitialized_copy(InputIterator first,
                                            InputIterator last,
                                            ForwardIterator result,
                                            T *)
{
    typedef typename __etype_traits<T>::is_POD_type is_POD;
    // why must give special case?
    return __uninitialized_copy_aux<InputIterator, ForwardIterator, T>(first, last, result,
                                                                       is_POD());
}
template <class InputIterator, class ForwardIterator>
inline ForwardIterator uninitialized_copy(InputIterator first,
                                          InputIterator last,
                                          ForwardIterator result)
{
    return __uninitialized_copy(first, last, result, value_type(result));
}

template <class ForwardIterator, class T, class T1>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &x, T1 *)
{
    typedef typename __etype_traits<T1>::is_POD_type is_POD;
    __uninitialized_fill_aux(first, last, x, is_POD());
}
template <class ForwardIterator, class T>
inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &x)
{
    __uninitialized_fill(first, last, x, value_type(first));
}

template <class ForwardIterator, class size, class T, class T1>
inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, size n, const T &x, T1 *)
{
    typedef typename __etype_traits<T1>::is_POD_type is_POD;
    return __uninitialized_fill_n_aux(first, n, x, is_POD());
}
template <class ForwardIterator, class size, class T>
inline ForwardIterator uninitialized_fill_n(ForwardIterator first, size n, const T &x)
{
    return __uninitialized_fill_n(first, n, x, value_type(first));
}

}  // namespace ESAF

#endif  // ememprocess_H
