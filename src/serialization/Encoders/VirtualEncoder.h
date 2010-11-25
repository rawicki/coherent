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

#ifndef VIRTUAL_ENCODER_H
#define VIRTUAL_ENCODER_H

#define DUMP_TYPEINFO 1

#include <boost/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include "Misc/TypeList.h"

#ifdef DUMP_TYPEINFO
#   include <cxxabi.h>
#   include <typeinfo>
#endif



namespace virtual_encoder_detail
{

//helpers for creating abstract encoders
template <typename T>
struct VirtualEncoderFunction
{
    virtual void operator() (const T&) = 0;
};

template <typename TypeList>
struct CreateEncoderAbs;

template <>
struct CreateEncoderAbs<ListHead>
{
};

template <typename CurrentType, typename ListTail>
struct CreateEncoderAbs<ListElem<CurrentType, ListTail> >
    : public VirtualEncoderFunction<CurrentType>,
      public CreateEncoderAbs<ListTail>
{
};


//helpers for creating impl
template <typename EncoderAbs, typename Encoder, typename TypeList>
struct CreateEncoderImpl;

template <typename EncoderAbs, typename Encoder>
struct CreateEncoderImpl<EncoderAbs, Encoder, ListHead>
    : public EncoderAbs
{
    CreateEncoderImpl(Encoder& enc) : enc_(enc) {}
    virtual ~CreateEncoderImpl() {}
protected:
    Encoder& enc_;
};

template <typename EncoderAbs, typename Encoder, typename CurrentType, typename ListTail>
struct CreateEncoderImpl<EncoderAbs, Encoder, ListElem<CurrentType, ListTail> >
    : public CreateEncoderImpl<EncoderAbs, Encoder, ListTail>
{
    typedef CreateEncoderImpl<EncoderAbs, Encoder, ListTail>    Super;
    CreateEncoderImpl(Encoder& enc) : Super(enc)
    {
    }
    virtual void operator() (const CurrentType& x)
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
        std::cout << "Encode(" << xtype_s << ") " << x << std::endl;
#endif
        enc_(x);
    }
};


//helpers for creating encoder
template <typename EncoderAbs, typename TypeList>
struct CreateEncoderType;

template <typename EncoderAbs>
struct CreateEncoderType<EncoderAbs, ListHead>
{
    CreateEncoderType() {}
    CreateEncoderType(EncoderAbs * impl) : impl_(impl) {}
    CreateEncoderType(boost::shared_ptr<EncoderAbs> impl) : impl_(impl) {}
protected:
    boost::shared_ptr<EncoderAbs> impl_;
};

template <typename EncoderAbs, typename CurrentType, typename ListTail>
struct CreateEncoderType<EncoderAbs, ListElem<CurrentType, ListTail> >
    : public CreateEncoderType<EncoderAbs, ListTail>
{
private:
    typedef CreateEncoderType<EncoderAbs, ListTail>     Super;
public:
    CreateEncoderType() {}
    CreateEncoderType(EncoderAbs * impl) : Super(impl) {}
    CreateEncoderType(boost::shared_ptr<EncoderAbs> impl) : Super(impl) {}
public:
    void encode(const CurrentType& x)
    {
        ((VirtualEncoderFunction<CurrentType>&)(*Super::impl_))(x);
    }
};

//finder
template <typename EncoderAbs, typename TypeList, typename T>
struct FindEncoderSuper;
//incomplete type error message if T can't be encoded with given EncoderType

    template <typename EncoderAbs, typename CurrentType, typename ListTail, bool found, typename T>
    struct FindEncoderSuperHelper;

    template <typename EncoderAbs, typename CurrentType, typename ListTail, typename T>
    struct FindEncoderSuperHelper<EncoderAbs, CurrentType, ListTail, true, T>
    {
        typedef CreateEncoderType<EncoderAbs, ListElem<CurrentType, ListTail> > Type;
    };
    template <typename EncoderAbs, typename CurrentType, typename ListTail, typename T>
    struct FindEncoderSuperHelper<EncoderAbs, CurrentType, ListTail, false, T>
    {
        typedef typename FindEncoderSuper<EncoderAbs, ListTail, T>::Type  Type;
    };

template <typename EncoderAbs, typename CurrentType, typename ListTail, typename T>
struct FindEncoderSuper<EncoderAbs, ListElem<CurrentType, ListTail>, T>
{
    typedef typename FindEncoderSuperHelper<EncoderAbs, CurrentType, ListTail, boost::is_same<CurrentType, T>::value, T>::Type   Type;
};

} //namespace



//create encoder set
template <typename TypeList>
struct CreateEncoderSet
{
    typedef virtual_encoder_detail::CreateEncoderAbs<TypeList> EncoderAbs;

    struct EncoderType
        : public virtual_encoder_detail::CreateEncoderType<EncoderAbs, TypeList>
    {
    private:
        typedef virtual_encoder_detail::CreateEncoderType<EncoderAbs, TypeList>     Super;
    public:
        EncoderType() {}    //empty impl_
        EncoderType(EncoderAbs * impl) : Super(impl) {}
        EncoderType(boost::shared_ptr<EncoderAbs> * impl) : Super(impl) {}

        template <typename T>
        EncoderType& operator() (const T& x)
        {
            virtual_encoder_detail::FindEncoderSuper<EncoderAbs, TypeList, T>::Type::encode(x);
            return *this;
        }
    };

    template <typename Encoder>
    struct EncoderImpl
        : public virtual_encoder_detail::CreateEncoderImpl<EncoderAbs, Encoder, TypeList>
    {
        typedef virtual_encoder_detail::CreateEncoderImpl<EncoderAbs, Encoder, TypeList>    Super;
        EncoderImpl(Encoder& enc) : Super(enc)
        {
        }
        virtual ~EncoderImpl() {}
    //protected:
    //    Encoder& enc_;
    };
};



#endif /* VIRTUAL_ENCODER_H */
