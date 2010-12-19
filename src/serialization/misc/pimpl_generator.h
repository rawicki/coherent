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
#include "misc/type_list.h"



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
struct CreateClassAbs;

template <template <class> class VirtualFnTmpl>
struct CreateClassAbs<VirtualFnTmpl, ListHead>
{
    virtual ~CreateClassAbs() {}
};

template <template <class> class VirtualFnTmpl, typename CurrentType, typename ListTail>
struct CreateClassAbs<VirtualFnTmpl, ListElem<CurrentType, ListTail> >
    : public VirtualFnTmpl<CurrentType>
    , public CreateClassAbs<VirtualFnTmpl, ListTail>
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
struct CreateClassType;

template <
    typename ClassAbs,
    template <class> class Qualifier,
    template <class> class TypeExtractor,
    template <class> class VirtualFnTmpl
>
struct CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListHead>
{
    CreateClassType() {}
    CreateClassType(ClassAbs * impl) : impl_(impl) {}
    CreateClassType(boost::shared_ptr<ClassAbs> impl) : impl_(impl) {}
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
struct CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListElem<CurrentType, ListTail> >
    : public CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListTail>
{
private:
    typedef CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListTail> Super;
    typedef typename TypeExtractor<CurrentType>::Type Type;
public:
    CreateClassType() {}
    CreateClassType(ClassAbs * impl) : Super(impl) {}
    CreateClassType(boost::shared_ptr<ClassAbs> impl) : Super(impl) {}
public:
    void foo(typename Qualifier<Type>::type x)
    {
        ((VirtualFnTmpl<CurrentType>&)(*Super::impl_))(x);
    }
    void foo(typename Qualifier<Type>::type x, uint32_t v)
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
struct FindClassSuper;
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
    struct FindClassSuperHelper;

    template <
        typename ClassAbs,
        template <class> class Qualifier,
        template <class> class TypeExtractor,
        template <class> class VirtualFnTmpl,
        typename CurrentType,
        typename ListTail,
        typename T
    >
    struct FindClassSuperHelper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, CurrentType, ListTail, true, T>
    {
        typedef CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListElem<CurrentType, ListTail> > Type;
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
    struct FindClassSuperHelper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, CurrentType, ListTail, false, T>
    {
        typedef typename FindClassSuper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListTail, T>::Type Type;
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
struct FindClassSuper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, ListElem<CurrentType, ListTail>, T>
{
    typedef
        typename FindClassSuperHelper<
                ClassAbs,
                Qualifier,
                TypeExtractor,
                VirtualFnTmpl,
                CurrentType,
                ListTail,
                boost::is_same<typename TypeExtractor<CurrentType>::Type, T>::value,
                T
            >::Type Type;
};


//default TypeExtractor
template <typename T>
struct Identity
{
    typedef T Type;
};


//create impl helper
template <
    typename ClassAbs,
    template <class> class ImplPolicy,
    template <class,class> class VirtualFnBody,
    typename TypeList
>
struct CreateClassImpl;

template <
    typename ClassAbs,
    template <class> class ImplPolicy,
    template <class,class> class VirtualFnBody
>
struct CreateClassImpl<ClassAbs, ImplPolicy, VirtualFnBody, ListHead>
{
    typedef ImplPolicy<ClassAbs>        Type;
};

template <
    typename ClassAbs,
    template <class> class ImplPolicy,
    template <class, class> class VirtualFnBody,
    typename CurrentType,
    typename ListTail
>
struct CreateClassImpl<ClassAbs, ImplPolicy, VirtualFnBody, ListElem<CurrentType, ListTail> >
{
    typedef typename CreateClassImpl<ClassAbs, ImplPolicy, VirtualFnBody, ListTail>::Type       Super;
    typedef typename VirtualFnBody<CurrentType, Super>::Type                                    Type;
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
struct CreatePImplSet
{
    typedef pimpl_detail::CreateClassAbs<VirtualFnTmpl, TypeList>      ClassAbs;

    struct ClassType
        : public pimpl_detail::CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList>
    {
    private:
        typedef pimpl_detail::CreateClassType<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList> Super;
    public:
        ClassType() {}
        ClassType(ClassAbs * impl) : Super(impl) {}
        ClassType(boost::shared_ptr<ClassAbs> impl) : Super(impl) {}

        template <typename T>
        void operator() (typename Qualifier<T>::type x)
        {
            pimpl_detail::FindClassSuper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList, T>::Type::foo(x);
        }
        template <typename T>
        void operator() (typename Qualifier<T>::type x, uint32_t v)
        {
            pimpl_detail::FindClassSuper<ClassAbs, Qualifier, TypeExtractor, VirtualFnTmpl, TypeList, T>::Type::foo(x, v);
        }
    };

    /* Implementation must have:
     *      - template <class Abs> class Policy - which serves as Base class, inherits only ClassAbs
     *      - template <class T, class Super> class Tmpl, where:
     *          * T - current type (before TypeExtractor, just list element),
     *          * Super - Tmpl generated type must inherit this class,
     *          * Super already inherits ClassAbs and Policy
     *          * Tmpl is sth like Class Factory: Tmpl<T, Super>::Type must indicate a class that should
     *              implement virtual functions from ClassAbs especially concerning type T
     */
    template <typename Implementation>
    struct ClassImpl
    {
        typedef
            typename pimpl_detail::CreateClassImpl<
                    ClassAbs,
                    Implementation::template Policy,
                    Implementation::template Tmpl,
                    TypeList
                >::Type Type;
    };


};

#endif /* MISC_PIMPL_GENERATOR_H */
