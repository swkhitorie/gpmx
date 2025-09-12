
#ifndef __EITERATOR_TRAITS_H_
#define __EITERATOR_TRAITS_H_

#include <stddef.h>

namespace ESAF 
{

struct einput_iterator_tag {};
struct eoutput_iterator_tag {};
struct eforward_iterator_tag : public einput_iterator_tag {};
struct ebidirectional_iterator_tag : public eforward_iterator_tag {};
struct erandom_access_iterator_tag : public ebidirectional_iterator_tag {};

template <class _Tp, class _Distance> struct input_iterator {
  typedef einput_iterator_tag iterator_category;
  typedef _Tp                 value_type;
  typedef _Distance           difference_type;
  typedef _Tp*                pointer;
  typedef _Tp&                reference;
};

struct output_iterator {
  typedef eoutput_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;
};

template <class _Tp, class _Distance> struct forward_iterator {
  typedef eforward_iterator_tag iterator_category;
  typedef _Tp                  value_type;
  typedef _Distance            difference_type;
  typedef _Tp*                 pointer;
  typedef _Tp&                 reference;
};


template <class _Tp, class _Distance> struct bidirectional_iterator {
  typedef ebidirectional_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
};

template <class _Tp, class _Distance> struct random_access_iterator {
  typedef erandom_access_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
};

// traits 获取各个迭代器的特性(相应类型)-----类型特性类,这是泛型类模板
template <class _Iterator>
struct eiterator_traits {
  typedef typename _Iterator::iterator_category iterator_category; // 迭代器类别
  typedef typename _Iterator::value_type        value_type;  // 迭代器解除引用后所得到的值的类型
  typedef typename _Iterator::difference_type   difference_type; // 两个迭代器之间的距离
  typedef typename _Iterator::pointer           pointer;      // 指向被迭代类型的指针
  typedef typename _Iterator::reference         reference;   // 被迭代类型的引用类型
};

// 针对原生指针(native pointer)而设计的 traits 偏特化版
template <class _Tp>
struct eiterator_traits<_Tp*> {
  typedef erandom_access_iterator_tag  iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;  // C++ 内建的 ptrdiff_t 类型
  typedef _Tp*                        pointer;
  typedef _Tp&                        reference;
};

// 针对原生之 pointer-to-const 而设计的 traits 偏特化版
template <class _Tp>
struct eiterator_traits<const _Tp*> {
  typedef erandom_access_iterator_tag  iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef const _Tp*                  pointer;
  typedef const _Tp&                  reference;
};

// 决定某个迭代器的类型-category 类别
template <class _Iter>
inline typename eiterator_traits<_Iter>::iterator_category
__iterator_category(const _Iter&)
{
  typedef typename eiterator_traits<_Iter>::iterator_category _Category;
  return _Category();
}

// 决定某个迭代器的类型-difference type
template <class _Iter>
inline typename eiterator_traits<_Iter>::difference_type*
__distance_type(const _Iter&)
{
  return static_cast<typename eiterator_traits<_Iter>::difference_type*>(0);
}

// 决定某个迭代器的类型-value_type
template <class _Iter>
inline typename eiterator_traits<_Iter>::value_type*
__value_type(const _Iter&)
{
  return static_cast<typename eiterator_traits<_Iter>::value_type*>(0);
}

// 封装 __iterator_category 函数
template <class _Iter>
inline typename eiterator_traits<_Iter>::iterator_category
iterator_category(const _Iter& __i) { return __iterator_category(__i); }

// 封装 __distance_type 函数
template <class _Iter>
inline typename eiterator_traits<_Iter>::difference_type*
distance_type(const _Iter& __i) { return __distance_type(__i); }

// 封装 value_type 函数
template <class _Iter>
inline typename eiterator_traits<_Iter>::value_type*
value_type(const _Iter& __i) { return __value_type(__i); }

#define __ITERATOR_CATEGORY(__i) __iterator_category(__i)
#define __DISTANCE_TYPE(__i)     __distance_type(__i)
#define __VALUE_TYPE(__i)        __value_type(__i)
  
}  // namespace ESAF

#endif  // eiterator_traits_H
