/*
 * (C) Copyright 2010 Tomasz Zolnowski
 * 
 * This file is part of CoherentDB.
 * 
 * CoherentDB is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CoherentDB is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with CoherentDB. If not, see
 * http://www.gnu.org/licenses/.
 */

#ifndef MISC_PIMPL_GENERATOR_H
#define MISC_PIMPL_GENERATOR_H

#include <boost/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <misc/type_list.h>

namespace coherent {
namespace misc {


//really ugly, sorry for that...
#define DEFINE_CONSTRUCTORS(ClassName, DerivedName) \
    ClassName() {} \
    template <typename T1> \
    ClassName(T1& t1) : DerivedName(t1) {} \
    template <typename T1, typename T2> \
    ClassName(T1& t1, T2& t2) : DerivedName(t1, t2) {} \
    template <typename T1, typename T2, typename T3> \
    ClassName(T1& t1, T2& t2, T3& t3) : DerivedName(t1, t2, t3) {} \
    template <typename T1, typename T2, typename T3, typename T4> \
    ClassName(T1& t1, T2& t2, T3& t3, T4& t4) : DerivedName(t1, t2, t3, t4) {} \
    template <typename T1, typename T2, typename T3, typename T4, typename T5> \
    ClassName(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5) : DerivedName(t1, t2, t3, t4, t5) {}


namespace pimpl_detail
{

//create abs helper
template <template <class> class VirtualFnTmpl, typename TypeList>
struct create_class_abs;

template <template <class> class VirtualFnTmpl>
struct create_class_abs<VirtualFnTmpl, list_head>
{
    virtual ~create_class_abs() {}
};

template <template <class> class VirtualFnTmpl, typename CurrentType, typename ListTail>
struct create_class_abs<VirtualFnTmpl, list_elem<CurrentType, ListTail> >
    : public VirtualFnTmpl<CurrentType>
    , public create_class_abs<VirtualFnTmpl, ListTail>
{
};


//create type helper
template <
    typename ClassAbs,
    template <class> class Qualifier,
    template <class> class TypeExtractor,
    template <class> class VirtualFnTmpl,
    typename TypeList
>
struct create_class_type;

template <
    typename ClassAbs,
    template <class> class Qualifier,
    template <class> class TypeExtractor,
    template <class> class VirtualFnTmpl
>
struct create_class_type<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, list_head>
{
    create_class_type() {}
    create_class_type(ClassAbs * impl) : impl_(impl) {}
    create_class_type(boost::shared_ptr<ClassAbs> impl) : impl_(impl) {}
protected:
    boost::shared_ptr<ClassAbs> impl_;
};

template <
    typename ClassAbs,
    template <class> class Qualifier,
    template <class> class TypeExtractor,
    template <class> class VirtualFnTmpl,
    typename CurrentType,
    typename ListTail
>
struct create_class_type<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, list_elem<CurrentType, ListTail> >
    : public create_class_type<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListTail>
{
private:
    typedef create_class_type<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListTail> Super;
    typedef typename TypeExtractor<CurrentType>::type type;
public:
    create_class_type() {}
    create_class_type(ClassAbs * impl) : Super(impl) {}
    create_class_type(boost::shared_ptr<ClassAbs> impl) : Super(impl) {}
public:
    void foo(typename Qualifier<type>::type x)
    {
        ((VirtualFnTmpl<CurrentType>&)(*Super::impl_))(x);
    }
    void foo(typename Qualifier<type>::type x, uint32_t v)
    {
        ((VirtualFnTmpl<CurrentType>&)(*Super::impl_))(x, v);
    }
};


//find base class helper
template <
    typename ClassAbs,
    template <class> class Qualifier,
    template <class> class TypeExtractor,
    template <class> class VirtualFnTmpl,
    typename TypeList,
    typename T
>
struct find_class_super;
//incomplete type error message if T is not an element of TypeList

    template <
        typename ClassAbs,
        template <class> class Qualifier,
        template <class> class TypeExtractor,
        template <class> class VirtualFnTmpl,
        typename CurrentType,
        typename ListTail,
        bool found,
        typename T
    >
    struct find_class_superHelper;

    template <
        typename ClassAbs,
        template <class> class Qualifier,
        template <class> class TypeExtractor,
        template <class> class VirtualFnTmpl,
        typename CurrentType,
        typename ListTail,
        typename T
    >
    struct find_class_superHelper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, CurrentType, ListTail, true, T>
    {
        typedef create_class_type<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, list_elem<CurrentType, ListTail> > type;
    };
    template <
        typename ClassAbs,
        template <class> class Qualifier,
        template <class> class TypeExtractor,
        template <class> class VirtualFnTmpl,
        typename CurrentType,
        typename ListTail,
        typename T
    >
    struct find_class_superHelper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, CurrentType, ListTail, false, T>
    {
        typedef typename find_class_super<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListTail, T>::type type;
    };

template <
    typename ClassAbs,
    template <class> class Qualifier,
    template <class> class TypeExtractor,
    template <class> class VirtualFnTmpl,
    typename CurrentType,
    typename ListTail,
    typename T
>
struct find_class_super<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, list_elem<CurrentType, ListTail>, T>
{
    typedef
        typename find_class_superHelper<
                ClassAbs,
                Qualifier,
                TypeExtractor,
                VirtualFnTmpl,
                CurrentType,
                ListTail,
                boost::is_same<typename TypeExtractor<CurrentType>::type, T>::value,
                T
            >::type type;
};


//default TypeExtractor
template <typename T>
struct Identity
{
    typedef T type;
};


//create impl helper
template <
    typename ClassAbs,
    template <class> class ImplPolicy,
    template <class,class> class VirtualFnBody,
    typename TypeList
>
struct create_class_impl;

template <
    typename ClassAbs,
    template <class> class ImplPolicy,
    template <class,class> class VirtualFnBody
>
struct create_class_impl<ClassAbs, ImplPolicy, VirtualFnBody, list_head>
{
    typedef ImplPolicy<ClassAbs>        type;
};

template <
    typename ClassAbs,
    template <class> class ImplPolicy,
    template <class, class> class VirtualFnBody,
    typename CurrentType,
    typename ListTail
>
struct create_class_impl<ClassAbs, ImplPolicy, VirtualFnBody, list_elem<CurrentType, ListTail> >
{
    typedef typename create_class_impl<ClassAbs, ImplPolicy, VirtualFnBody, ListTail>::type       Super;
    typedef typename VirtualFnBody<CurrentType, Super>::type                                    type;
};


} //namespace



/* template params:
 *  - typename TypeList, list of types to encode/decode possibly with functors Version/Both etc.
 *  - template <T> VirtualFnTmpl: use to create single prototype struct for a given type T (TypeList elem)
 *  - template <T> Qualifier: define qualifiers for calls in ClassType (should have `type' adding needed const/reference)
 *  - template <T> TypeExtractor: extract proper type from TypeList element, e.g. extract T from Version<T>, used
 *      to compare and find proper type on list when call to ClassType,
 */
template <
    typename TypeList,
    template <class> class VirtualFnTmpl,
    template <class> class Qualifier,
    template <class> class TypeExtractor = pimpl_detail::Identity
>
struct create_pimpl_set
{
    typedef pimpl_detail::create_class_abs<VirtualFnTmpl, TypeList>      class_abs;

    struct class_type
        : public pimpl_detail::create_class_type<class_abs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList>
    {
    private:
        typedef pimpl_detail::create_class_type<class_abs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList> Super;
    public:
        class_type() {}
        class_type(class_abs * impl) : Super(impl) {}
        class_type(boost::shared_ptr<class_abs> impl) : Super(impl) {}

        template <typename T>
        void operator() (typename Qualifier<T>::type x)
        {
            pimpl_detail::find_class_super<class_abs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList, T>::type::foo(x);
        }
        template <typename T>
        void operator() (typename Qualifier<T>::type x, uint32_t v)
        {
            pimpl_detail::find_class_super<class_abs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList, T>::type::foo(x, v);
        }
    };

    /* Implementation must have:
     *      - template <class Abs> class Policy - which serves as Base class, inherits only ClassAbs
     *      - template <class T, class Super> class Tmpl, where:
     *          * T - current type (before TypeExtractor, just list element),
     *          * Super - Tmpl generated type must inherit this class,
     *          * Super already inherits ClassAbs and Policy
     *          * Tmpl is sth like Class Factory: Tmpl<T, Super>::type must indicate a class that should
     *              implement virtual functions from ClassAbs especially concerning type T
     */
    template <typename Implementation>
    struct class_impl
    {
        typedef
            typename pimpl_detail::create_class_impl<
                    class_abs,
                    Implementation::template policy,
                    Implementation::template tmpl,
                    TypeList
                >::type type;
    };


};


} // namespace misc
} // namespace coherent

#endif /* MISC_PIMPL_GENERATOR_H */
