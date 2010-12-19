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

#ifndef VIRTUAL_DECODER_H
#define VIRTUAL_DECODER_H

#include <boost/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include "misc/type_list.h"

#ifdef DUMP_TYPEINFO
#   include "misc/demangle.h"
#endif



namespace virtual_decoder_detail
{

//helpers for creating abstract decoders
template <typename T>
struct VirtualDecoderFunction
{
    virtual void operator() (T&) = 0;
};

template <typename TypeList>
struct CreateDecoderAbs;

template <>
struct CreateDecoderAbs<list_head>
{
};

template <typename CurrentType, typename ListTail>
struct CreateDecoderAbs<list_elem<CurrentType, ListTail> >
    : public VirtualDecoderFunction<CurrentType>,
      public CreateDecoderAbs<ListTail>
{
};


//helpers for creating impl
template <typename DecoderAbs, typename Decoder, typename TypeList>
struct CreateDecoderImpl;

template <typename DecoderAbs, typename Decoder>
struct CreateDecoderImpl<DecoderAbs, Decoder, list_head>
    : public DecoderAbs
{
    CreateDecoderImpl(Decoder& dec) : dec_(dec) {}
    virtual ~CreateDecoderImpl() {}
protected:
    Decoder& dec_;
};

template <typename DecoderAbs, typename Decoder, typename CurrentType, typename ListTail>
struct CreateDecoderImpl<DecoderAbs, Decoder, list_elem<CurrentType, ListTail> >
    : public CreateDecoderImpl<DecoderAbs, Decoder, ListTail>
{
    typedef CreateDecoderImpl<DecoderAbs, Decoder, ListTail>    Super;
    CreateDecoderImpl(Decoder& dec) : Super(dec)
    {
    }
    virtual void operator() (CurrentType& x)
    {
        dec_(x);
#ifdef DUMP_TYPEINFO
        std::cout << "Decoded(" << demangle<CurrentType>() << ") " << x << std::endl;
#endif
    }
};


//helpers for creating decoder
template <typename DecoderAbs, typename TypeList>
struct CreateDecoderType;

template <typename DecoderAbs>
struct CreateDecoderType<DecoderAbs, list_head>
{
    CreateDecoderType() {}
    CreateDecoderType(DecoderAbs * impl) : impl_(impl) {}
    CreateDecoderType(boost::shared_ptr<DecoderAbs> impl) : impl_(impl) {}
protected:
    boost::shared_ptr<DecoderAbs> impl_;
};

template <typename DecoderAbs, typename CurrentType, typename ListTail>
struct CreateDecoderType<DecoderAbs, list_elem<CurrentType, ListTail> >
    : public CreateDecoderType<DecoderAbs, ListTail>
{
private:
    typedef CreateDecoderType<DecoderAbs, ListTail>     Super;
public:
    CreateDecoderType() {}
    CreateDecoderType(DecoderAbs * impl) : Super(impl) {}
    CreateDecoderType(boost::shared_ptr<DecoderAbs> impl) : Super(impl) {}
public:
    void decode(CurrentType& x)
    {
        ((VirtualDecoderFunction<CurrentType>&)(*Super::impl_))(x);
    }
};

//finder
template <typename DecoderAbs, typename TypeList, typename T>
struct FindDecoderSuper;

    template <typename DecoderAbs, typename CurrentType, typename ListTail, bool found, typename T>
    struct FindDecoderSuperHelper;

    template <typename DecoderAbs, typename CurrentType, typename ListTail, typename T>
    struct FindDecoderSuperHelper<DecoderAbs, CurrentType, ListTail, true, T>
    {
        typedef CreateDecoderType<DecoderAbs, list_elem<CurrentType, ListTail> > Type;
    };
    template <typename DecoderAbs, typename CurrentType, typename ListTail, typename T>
    struct FindDecoderSuperHelper<DecoderAbs, CurrentType, ListTail, false, T>
    {
        typedef typename FindDecoderSuper<DecoderAbs, ListTail, T>::Type  Type;
    };

template <typename DecoderAbs, typename CurrentType, typename ListTail, typename T>
struct FindDecoderSuper<DecoderAbs, list_elem<CurrentType, ListTail>, T>
{
    typedef typename FindDecoderSuperHelper<DecoderAbs, CurrentType, ListTail, boost::is_same<CurrentType, T>::value, T>::Type   Type;
};

} //namespace



//create decoder set
template <typename TypeList>
struct CreateDecoderSet
{
    typedef virtual_decoder_detail::CreateDecoderAbs<TypeList> DecoderAbs;

    struct DecoderType
        : public virtual_decoder_detail::CreateDecoderType<DecoderAbs, TypeList>
    {
    private:
        typedef virtual_decoder_detail::CreateDecoderType<DecoderAbs, TypeList>     Super;
    public:
        DecoderType() {}    //empty impl_
        DecoderType(DecoderAbs * impl) : Super(impl) {}
        DecoderType(boost::shared_ptr<DecoderAbs> * impl) : Super(impl) {}

        template <typename T>
        DecoderType& operator() (T& x)
        {
            virtual_decoder_detail::FindDecoderSuper<DecoderAbs, TypeList, T>::Type::decode(x);
            return *this;
        }
    };

    template <typename Decoder>
    struct DecoderImpl
        : public virtual_decoder_detail::CreateDecoderImpl<DecoderAbs, Decoder, TypeList>
    {
        typedef virtual_decoder_detail::CreateDecoderImpl<DecoderAbs, Decoder, TypeList>    Super;
        DecoderImpl(Decoder& dec) : Super(dec)
        {
        }
        virtual ~DecoderImpl() {}
    //protected:
    //    Decoder& dec_;
    };
};



#endif /* VIRTUAL_DECODER_H */
