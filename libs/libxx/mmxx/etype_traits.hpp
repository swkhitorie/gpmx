
#ifndef _ETYPE_TRAITS_H_
#define _ETYPE_TRAITS_H_

namespace ESTL
{

// clang-format off
class __etrue_type {};
class __efalse_type {};

#define __E_TEMPLATE_NULL template <>

/*!
 * @brief copied from STL type_traits
 */
template <class type>
struct __etype_traits
{
  public:
    typedef __etrue_type this_dummy_member_must_be_first;

    typedef __efalse_type has_trivial_default_constructor;
    typedef __efalse_type has_trivial_copy_constructor;
    typedef __efalse_type has_trivial_assignment_constructor;
    typedef __efalse_type has_trivial_destructor;
    typedef __efalse_type is_POD_type;  // Plain Old Data
};

__E_TEMPLATE_NULL struct __etype_traits<char>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<signed char>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<unsigned char>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<short>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<unsigned short>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<int>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<unsigned int>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<long>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<unsigned long>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<float>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<double>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<long double>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

template <class T>
struct __etype_traits<T *>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<char *>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<signed char *>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

__E_TEMPLATE_NULL struct __etype_traits<unsigned char *>
{
  public:
    typedef __etrue_type has_trivial_default_constructor;
    typedef __etrue_type has_trivial_copy_constructor;
    typedef __etrue_type has_trivial_assignment_operator;
    typedef __etrue_type has_trivial_destructor;
    typedef __etrue_type is_POD_type;
};

}  // namespace ESAF

#endif  // etype_traits_H
