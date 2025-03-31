
#ifndef _ECONSTRUCTOR_H_
#define _ECONSTRUCTOR_H_

namespace ESTL
{

/*!
 * @brief memory allocator ----
 * basical tool of constructor and destroyer
 */

/*!
 * @brief get class address prevents operators & from being overloaded
 */
template <typename _Tp>
inline _Tp *__addressof(_Tp &__r)
{
    return reinterpret_cast<_Tp *>(
        &const_cast<char &>(reinterpret_cast<const volatile char &>(__r)));
}

template <typename _Tp>
inline _Tp *addressof(_Tp &__r)
{
    return __addressof(__r);
}

#if __cplusplus >= 201103L
template <class _T, class... ARGS>
inline void _construct(_T *__p, ARGS... args)
{
    /* global namepsace */
    ::new (static_cast<void *>(&*__p)) _T(args...);
}
#else
template <class _T1, class _T2>
inline void _construct(_T1 *p, const _T2 &value)
{
    ::new (p) _T1(value);  // placement new
}
#endif

template <class _T>
inline void _construct_novalue(_T *__p)
{
    ::new (static_cast<void *>(&*__p)) _T();
}

template <class T>
inline void _destroy(T *pointer)
{
    pointer->~T();
}

// range-based destroy interface
template <bool>
struct _destroy_aux
{
    template <typename _ForwardIterator>
    static void __destroy(_ForwardIterator __first, _ForwardIterator __last)
    {
        for (; __first != __last; ++__first)
            _destroy(__addressof(*__first));
    }
};

template <>
struct _destroy_aux<true>
{
    template <typename _ForwardIterator>
    static void __destroy(_ForwardIterator, _ForwardIterator)
    {}
};

template <bool>
struct _destroy_n_aux
{
    template <typename _ForwardIterator, typename _Size>
    static _ForwardIterator __destroy_n(_ForwardIterator __first, _Size __count)
    {
        for (; __count > 0; (void)++__first, --__count)
            _destroy(__addressof(*__first));
        return __first;
    }
};

template <>
struct _destroy_n_aux<true>
{
    template <typename _ForwardIterator, typename _Size>
    static _ForwardIterator __destroy_n(_ForwardIterator __first, _Size __count)
    {
        return __first;
    }
};

template <typename _ForwardIterator>
inline void _destroy(_ForwardIterator __first, _ForwardIterator __last)
{
    _destroy_aux<false>::__destroy(__first, __last);
}

template <typename _ForwardIterator, typename _Size>
inline _ForwardIterator _destroy_n(_ForwardIterator __first, _Size __count)
{
    return _destroy_n_aux<false>::__destroy_n(__first, __count);
}

template <typename _ForwardIterator>
inline void destroy(_ForwardIterator __first, _ForwardIterator __last)
{
    _destroy(__first, __last);
}

template <typename _ForwardIterator, typename _Size>
inline _ForwardIterator destroy_n(_ForwardIterator __first, _Size __count)
{
    return _destroy_n(__first, __count);
}

}  // namespace ESAF

#endif  // econstructor_H
