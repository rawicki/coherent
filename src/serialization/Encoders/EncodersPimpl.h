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

#ifndef ENCODERS_PIMPL_H
#define ENCODERS_PIMPL_H

#include <boost/type_traits.hpp>
#include "Misc/PImplGenerator.h"

#ifdef DUMP_TYPEINFO
#   include <cxxabi.h>
#   include <typeinfo>
#endif


namespace detail
{
    template <typename T> struct Default {};
    template <typename T> struct Version {};
    template <typename T> struct Both {};

    template <typename T> struct TypeExtractor { typedef T Type; };    //default
    template <typename T> struct TypeExtractor< Default<T> > { typedef T Type; };
    template <typename T> struct TypeExtractor< Version<T> > { typedef T Type; };
    template <typename T> struct TypeExtractor< Both<T> >    { typedef T Type; };


    //encoder
    template <typename T>
    struct VirtualEncoderFunction
    {
        virtual void operator() (const T&) = 0;
    };
    template <typename T>
    struct VirtualEncoderFunction< Default<T> >
    {
        virtual void operator() (const T&) = 0;
    };
    template <typename T>
    struct VirtualEncoderFunction< Version<T> >
    {
        virtual void operator() (const T&, uint32_t) = 0;
    };
    template <typename T>
    struct VirtualEncoderFunction< Both<T> >
    {
        virtual void operator() (const T&) = 0;
        virtual void operator() (const T&, uint32_t) = 0;
    };

    template <typename T>
    struct EncoderQualifier
    {
        typedef typename boost::add_const<typename boost::add_reference<T>::type>::type type;
    };

    //decoder
    template <typename T>
    struct VirtualDecoderFunction
    {
        virtual void operator() (T&) = 0;
    };
    template <typename T>
    struct VirtualDecoderFunction< Default<T> >
    {
        virtual void operator() (T&) = 0;
    };
    template <typename T>
    struct VirtualDecoderFunction< Version<T> >
    {
        virtual void operator() (T&, uint32_t) = 0;
    };
    template <typename T>
    struct VirtualDecoderFunction< Both<T> >
    {
        virtual void operator() (T&) = 0;
        virtual void operator() (T&, uint32_t) = 0;
    };

    template <typename T>
    struct DecoderQualifier
    {
        typedef typename boost::add_reference<T>::type type;
    };



    template <template <class> class Qualifier>
    struct RefPolicy
    {
        template <typename Derived>
        struct Policy
        {
            Policy(Derived& foo) : foo_(foo)
            {
            }
            Derived& getFoo()
            {
                return foo_;
            }
        private:
            Derived& foo_;
        };

        //TODO, wyci±æ typeinfo i zrobiæ z tego jedn± funkcjê
        template <typename T, typename Super>
        struct Tmpl : public Super
        {
            DEFINE_CONSTRUCTORS(Tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x)
            {
#ifdef DUMP_TYPEINFO
                int ret = 0;
                std::string xtype_s;
                char * xtype = abi::__cxa_demangle(typeid(x).name(), NULL, NULL, &ret);
                if (ret==0) {
                    xtype_s = xtype;
                    free(xtype);
                }
                else {
                    xtype_s = std::string("Failed[") + typeid(x).name() + "]";
                }
#endif
                Super::getFoo()(x);
#ifdef DUMP_TYPEINFO
                std::cout << "Decoded(" << xtype_s << ") " << x << std::endl;
#endif
            }
        };
        template <typename T, typename Super>
        struct Tmpl< Version<T>, Super > : public Super
        {
            DEFINE_CONSTRUCTORS(Tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x, uint32_t v)
            {
#ifdef DUMP_TYPEINFO
                int ret = 0;
                std::string xtype_s;
                char * xtype = abi::__cxa_demangle(typeid(x).name(), NULL, NULL, &ret);
                if (ret==0) {
                    xtype_s = xtype;
                    free(xtype);
                }
                else {
                    xtype_s = std::string("Failed[") + typeid(x).name() + "]";
                }
#endif
                Super::getFoo()(x, v);
#ifdef DUMP_TYPEINFO
                std::cout << "Decoded_v(" << xtype_s << ")(" << v << ") " << x << std::endl;
#endif
            }
        };
        //TODO dopisaæ pozosta³e specjalizacje Tmpl
    };

    //TODO dodaæ politykê, gdzie dekoder/enkoder trzymany jest jako pole, a nie tylko jako referencja
}


template <typename TypeList>
struct CreateEncoderSet
{
    typedef CreatePImplSet<
            TypeList,
            detail::VirtualEncoderFunction,
            detail::EncoderQualifier,
            detail::TypeExtractor
        >
            PImplSet;

    typedef typename PImplSet::ClassAbs     EncoderAbs;
    typedef typename PImplSet::ClassType    EncoderType;

    //template <typename Encoder> struct EncoderImpl {};
    //TODO encodertype - tak jak w decoderze,
    //
};


template <typename TypeList>
struct CreateDecoderSet
{
    typedef CreatePImplSet<
            TypeList,
            detail::VirtualDecoderFunction,
            detail::DecoderQualifier,
            detail::TypeExtractor
        >
            PImplSet;

    typedef typename PImplSet::ClassAbs     DecoderAbs;

    struct DecoderType : public PImplSet::ClassType
    {
        typedef typename PImplSet::ClassType Super;
        DecoderType() {}
        DecoderType(DecoderAbs * impl) : Super(impl) {}
        DecoderType(boost::shared_ptr<DecoderAbs> impl) : Super(impl) {}

        template <typename T>
        DecoderType& operator() (T & x)
        {
            Super::template operator()<T>(x);
            return *this;
        }
        template <typename T>
        DecoderType& operator() (T & x, uint32_t v)
        {
            Super::template operator()<T>(x, v);
            return *this;
        }
    };

    /*template <typename DecoderPolicy>
    struct DecoderImpl
    {
        typedef typename PImplSet::template ClassImpl<DecoderPolicy> Type;
    };*/

    //TODO, poni¿sze jest tylko dla kompatybilno¶ci wstecznej; standardowo u¿ywamy powy¿szego
    template <typename Decoder>
    struct DecoderImpl
    {
        typedef typename PImplSet::template ClassImpl<
                    detail::RefPolicy<detail::DecoderQualifier>::template Policy<Decoder>,
                    detail::RefPolicy<detail::DecoderQualifier>::template Tmpl
                > Type;
    };
};


#endif /* ENCODERS_PIMPL_H */
