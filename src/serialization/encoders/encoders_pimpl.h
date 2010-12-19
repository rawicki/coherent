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
#include "misc/pimpl_generator.h"

#ifdef DUMP_TYPEINFO
#   include "misc/demangle.h"
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
        //TODO sprawdzic czy wystarczy dziedziczenie po Version i Default ?
    };

    template <typename T>
    struct EncoderQualifier
    {
        //typedef typename boost::add_const<typename boost::add_reference<T>::type>::type type;
        typedef const T & type;
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
        //typedef typename boost::add_reference<T>::type type;
        typedef T & type;
    };


    template <template <class> class Qualifier>
    struct DefaultImplementation
    {
#ifdef DUMP_TYPEINFO
        template <typename T> static void process_info(typename Qualifier<T>::type x)
        {
            if (boost::is_same<typename Qualifier<T>::type, const T&>::value)
                std::cout << "encoding";
            else std::cout << "decoding";
            std::cout << "(" << demangle<T>() << ") " << x << std::endl;
        }
        template <typename T> static void process_info_v(typename Qualifier<T>::type x, uint32_t v)
        {
            if (boost::is_same<typename Qualifier<T>::type, const T&>::value)
                std::cout << "encoding_v";
            else std::cout << "decoding_v";
            std::cout << "(" << demangle<T>() << ")(" << v << ") " << x << std::endl;
        }
#   define DUMP_RESULT(T, x)        process_info<T>(x)
#   define DUMP_RESULT_V(T, x, v)   process_info_v<T>(x, v)
#else
#   define DUMP_RESULT(T, x)
#   define DUMP_RESULT_V(T, x, v)
#endif
        template <typename T, typename Super>
        struct Tmpl : public Super
        {
            DEFINE_CONSTRUCTORS(Tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x)
            {
                Super::getFoo()(x);
                DUMP_RESULT(T, x);
            }
        };
        template <typename T, typename Super>
        struct Tmpl< Default<T>, Super > : public Super
        {
            DEFINE_CONSTRUCTORS(Tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x)
            {
                Super::getFoo()(x);
                DUMP_RESULT(T, x);
            }
        };
        template <typename T, typename Super>
        struct Tmpl< Version<T>, Super > : public Super
        {
            DEFINE_CONSTRUCTORS(Tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x, uint32_t v)
            {
                Super::getFoo()(x, v);
                DUMP_RESULT_V(T, x, v);
            }
        };
        template <typename T, typename Super>
        struct Tmpl< Both<T>, Super > : public Super
        {
            //TODO sprawdzic czy wystarczy dziedziczenie po powyzszych?
            DEFINE_CONSTRUCTORS(Tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x)
            {
                Super::getFoo()(x);
                DUMP_RESULT(T, x);
            }
            virtual void operator() (typename Qualifier<T>::type x, uint32_t v)
            {
                Super::getFoo()(x, v);
                DUMP_RESULT_V(T, x, v);
            }
        };
    };

    template <template <class> class Qualifier, typename Derived>
    struct RefPolicy
    {
        template <typename Abs>
        struct Policy : public Abs
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

        template <typename T, typename Super>
        struct Tmpl
        {
            typedef typename DefaultImplementation<Qualifier>::template Tmpl<T, Super> Type;
        };
    };

    template <template <class> class Qualifier, typename Derived>
    struct FieldPolicy
    {
        template <typename Abs>
        struct Policy : public Abs
        {
            DEFINE_CONSTRUCTORS(Policy, foo_)

            Derived& getFoo()
            {
                return foo_;
            }
        private:
            Derived foo_;
        };

        template <typename T, typename Super>
        struct Tmpl
        {
            typedef typename DefaultImplementation<Qualifier>::template Tmpl<T, Super> Type;
        };
    };
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

    struct EncoderType : public PImplSet::ClassType
    {
        typedef typename PImplSet::ClassType Super;
        EncoderType() {}
        EncoderType(EncoderAbs * impl) : Super(impl) {}
        EncoderType(boost::shared_ptr<EncoderAbs> impl) : Super(impl) {}

        template <typename T>
        EncoderType& operator() (const T & x)
        {
            Super::template operator()<T>(x);
            return *this;
        }
        template <typename T>
        EncoderType& operator() (const T & x, uint32_t v)
        {
            Super::template operator()<T>(x, v);
            return *this;
        }
    };

    template <typename EncoderPolicy>
    struct CreateEncoderImpl
    {
        typedef typename PImplSet::template ClassImpl<EncoderPolicy>::Type Type;
    };

    //tak jak w decoderze zostawione dla kompatybilno¶ci wstecznej
    template <typename Encoder>
    struct EncoderImpl
    {
        typedef typename PImplSet::template ClassImpl<detail::RefPolicy<detail::EncoderQualifier, Encoder> >::Type Type;
    };
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

    template <typename DecoderPolicy>
    struct CreateDecoderImpl
    {
        typedef typename PImplSet::template ClassImpl<DecoderPolicy>::Type Type;
    };

    //poni¿sze zostaje jedynie dla kompatybilno¶ci wstecznej, jedyn± ró¿nic± jest dodanie `::Type'
    //w wiêkszo¶ci przypadków zalecane jest u¿ywanie CreateDecoderImpl
    template <typename Decoder>
    struct DecoderImpl
    {
        typedef typename PImplSet::template ClassImpl<detail::RefPolicy<detail::DecoderQualifier, Decoder> >::Type Type;
    };
};


#endif /* ENCODERS_PIMPL_H */
