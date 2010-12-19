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
    template <typename T> struct __default {};
    template <typename T> struct __version {};
    template <typename T> struct __both {};

    template <typename T> struct type_extractor { typedef T type; };    //default
    template <typename T> struct type_extractor< __default<T> > { typedef T type; };
    template <typename T> struct type_extractor< __version<T> > { typedef T type; };
    template <typename T> struct type_extractor< __both<T> >    { typedef T type; };


    //encoder
    template <typename T>
    struct virtual_encoder_function
    {
        virtual void operator() (const T&) = 0;
    };
    template <typename T>
    struct virtual_encoder_function< __default<T> >
    {
        virtual void operator() (const T&) = 0;
    };
    template <typename T>
    struct virtual_encoder_function< __version<T> >
    {
        virtual void operator() (const T&, uint32_t) = 0;
    };
    template <typename T>
    struct virtual_encoder_function< __both<T> >
    {
        virtual void operator() (const T&) = 0;
        virtual void operator() (const T&, uint32_t) = 0;
        //TODO sprawdzic czy wystarczy dziedziczenie po __version i __default ?
    };

    template <typename T>
    struct encoder_qualifier
    {
        //typedef typename boost::add_const<typename boost::add_reference<T>::type>::type type;
        typedef const T & type;
    };

    //decoder
    template <typename T>
    struct virtual_decoder_function
    {
        virtual void operator() (T&) = 0;
    };
    template <typename T>
    struct virtual_decoder_function< __default<T> >
    {
        virtual void operator() (T&) = 0;
    };
    template <typename T>
    struct virtual_decoder_function< __version<T> >
    {
        virtual void operator() (T&, uint32_t) = 0;
    };
    template <typename T>
    struct virtual_decoder_function< __both<T> >
    {
        virtual void operator() (T&) = 0;
        virtual void operator() (T&, uint32_t) = 0;
    };

    template <typename T>
    struct decoder_qualifier
    {
        //typedef typename boost::add_reference<T>::type type;
        typedef T & type;
    };


    template <template <class> class Qualifier>
    struct default_implementation
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
        struct tmpl : public Super
        {
            DEFINE_CONSTRUCTORS(tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x)
            {
                Super::getFoo()(x);
                DUMP_RESULT(T, x);
            }
        };
        template <typename T, typename Super>
        struct tmpl< __default<T>, Super > : public Super
        {
            DEFINE_CONSTRUCTORS(tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x)
            {
                Super::getFoo()(x);
                DUMP_RESULT(T, x);
            }
        };
        template <typename T, typename Super>
        struct tmpl< __version<T>, Super > : public Super
        {
            DEFINE_CONSTRUCTORS(tmpl, Super)

            virtual void operator() (typename Qualifier<T>::type x, uint32_t v)
            {
                Super::getFoo()(x, v);
                DUMP_RESULT_V(T, x, v);
            }
        };
        template <typename T, typename Super>
        struct tmpl< __both<T>, Super > : public Super
        {
            //TODO sprawdzic czy wystarczy dziedziczenie po powyzszych?
            DEFINE_CONSTRUCTORS(tmpl, Super)

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
    struct ref_policy
    {
        template <typename Abs>
        struct policy : public Abs
        {
            policy(Derived& foo) : foo_(foo)
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
        struct tmpl
        {
            typedef typename default_implementation<Qualifier>::template tmpl<T, Super> type;
        };
    };

    template <template <class> class Qualifier, typename Derived>
    struct field_policy
    {
        template <typename Abs>
        struct policy : public Abs
        {
            DEFINE_CONSTRUCTORS(policy, foo_)

            Derived& getFoo()
            {
                return foo_;
            }
        private:
            Derived foo_;
        };

        template <typename T, typename Super>
        struct tmpl
        {
            typedef typename default_implementation<Qualifier>::template tmpl<T, Super> type;
        };
    };
} //namespace detail


template <typename TypeList>
struct create_encoder_set
{
    typedef create_pimpl_set<
            TypeList,
            detail::virtual_encoder_function,
            detail::encoder_qualifier,
            detail::type_extractor
        >
            pimpl_set;

    typedef typename pimpl_set::class_abs     encoder_abs;

    struct encoder_type : public pimpl_set::class_type
    {
        typedef typename pimpl_set::class_type Super;
        encoder_type() {}
        encoder_type(encoder_abs * impl) : Super(impl) {}
        encoder_type(boost::shared_ptr<encoder_abs> impl) : Super(impl) {}

        template <typename T>
        encoder_type& operator() (const T & x)
        {
            Super::template operator()<T>(x);
            return *this;
        }
        template <typename T>
        encoder_type& operator() (const T & x, uint32_t v)
        {
            Super::template operator()<T>(x, v);
            return *this;
        }
    };

    template <typename EncoderPolicy>
    struct create_encoder_impl
    {
        typedef typename pimpl_set::template class_impl<EncoderPolicy>::type type;
    };

    //tak jak w decoderze zostawione dla kompatybilno¶ci wstecznej
    template <typename Encoder>
    struct encoder_impl
    {
        typedef typename pimpl_set::template class_impl<detail::ref_policy<detail::encoder_qualifier, Encoder> >::type type;
    };
};


template <typename TypeList>
struct create_decoder_set
{
    typedef create_pimpl_set<
            TypeList,
            detail::virtual_decoder_function,
            detail::decoder_qualifier,
            detail::type_extractor
        >
            pimpl_set;

    typedef typename pimpl_set::class_abs     decoder_abs;

    struct decoder_type : public pimpl_set::class_type
    {
        typedef typename pimpl_set::class_type Super;
        decoder_type() {}
        decoder_type(decoder_abs * impl) : Super(impl) {}
        decoder_type(boost::shared_ptr<decoder_abs> impl) : Super(impl) {}

        template <typename T>
        decoder_type& operator() (T & x)
        {
            Super::template operator()<T>(x);
            return *this;
        }
        template <typename T>
        decoder_type& operator() (T & x, uint32_t v)
        {
            Super::template operator()<T>(x, v);
            return *this;
        }
    };

    template <typename DecoderPolicy>
    struct create_decoder_impl
    {
        typedef typename pimpl_set::template class_impl<DecoderPolicy>::type type;
    };

    //poni¿sze zostaje jedynie dla kompatybilno¶ci wstecznej, jedyn± ró¿nic± jest dodanie `::type'
    //w wiêkszo¶ci przypadków zalecane jest u¿ywanie CreateDecoderImpl
    template <typename Decoder>
    struct decoder_impl
    {
        typedef typename pimpl_set::template class_impl<detail::ref_policy<detail::decoder_qualifier, Decoder> >::type type;
    };
};


#endif /* ENCODERS_PIMPL_H */
