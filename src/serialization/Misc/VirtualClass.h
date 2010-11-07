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

#ifndef VIRTUAL_CLASS_H
#define VIRTUAL_CLASS_H

#include <boost/shared_ptr.hpp>
#include <inttypes.h>


struct ListHead
{
};

template <typename This, typename Next>
struct ListElem
{
    typedef This Type;
};

template <typename T1> struct makeList1 {
    typedef ListElem<T1, ListHead> value;
};
template <typename T1, typename T2> struct makeList2 {
    typedef ListElem<T1, typename makeList1<T2>::value> value;
};
template <typename T1, typename T2, typename T3> struct makeList3 {
    typedef ListElem<T1, typename makeList2<T2, T3>::value> value;
};
template <typename T1, typename T2, typename T3, typename T4> struct makeList4 {
    typedef ListElem<T1, typename makeList3<T2, T3, T4>::value> value;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5> struct makeList5 {
    typedef ListElem<T1, typename makeList4<T2, T3, T4, T5>::value> value;
};



template <typename F, typename TypeList>
struct Fold;

template <typename F, typename CurrentType, typename TypeList>
struct Fold<F, ListElem<CurrentType, TypeList> >
{
    void operator() (F& f) const
    {
        if (f.template process<CurrentType>())
            return;
        Fold<F, TypeList>() (f);
    }
};

template <typename F>
struct Fold<F, ListHead>
{
    void operator() (F&) const
    {
        throw "End of List!";
    }
};


template <typename BaseType, typename TypeList>
struct Virtual
{
    typedef typename BaseType::TagType TagType;
private:
    template <typename Encoder>
    struct TaggedEncoder
    {
        TaggedEncoder(Encoder& enc, boost::shared_ptr<BaseType> ptr) : enc_(enc), ptr_(ptr), tag_(ptr->getTag())
        {
        }
        template <typename Type>
        bool process() {
            if (Type::TAG == tag_) {
                enc_(Type::TAG);
                enc_(static_cast<const Type&>(*ptr_));
                return true;
            }
            return false;
        }
        Encoder& enc_;
        boost::shared_ptr<BaseType> ptr_;
        TagType tag_;
    };
    template <typename Decoder>
    struct TaggedDecoder
    {
        TaggedDecoder(Decoder& dec, boost::shared_ptr<BaseType>& ptr, TagType tag) : dec_(dec), ptr_(ptr), tag_(tag)
        {
        }
        template <typename Type>
        bool process() {
            if (Type::TAG == tag_) {
                ptr_.reset(new Type());
                dec_(static_cast<Type&>(*ptr_));
                return true;
            }
            return false;
        }
        Decoder& dec_;
        boost::shared_ptr<BaseType>& ptr_;
        TagType tag_;
    };
public:
    template <typename Encoder>
    void encode(Encoder& enc) const
    {
        if (!ptr_.get()) {
            enc(TagType());
            return;
        }
        TaggedEncoder<Encoder> te(enc, ptr_);
        Fold<TaggedEncoder<Encoder>, TypeList>()(te);
    }

    template <typename Decoder>
    void decode(Decoder& dec)
    {
        TagType tag;
        dec(tag);
        if (tag==TagType()) {
            ptr_.reset();
            return;
        }
        TaggedDecoder<Decoder> td(dec, ptr_, tag);
        Fold<TaggedDecoder<Decoder>, TypeList>()(td);
    }

    const BaseType * get_ptr() const
    {
        return ptr_.get();
    }

    const BaseType& get() const
    {
        return *ptr_;
    }

    void set(BaseType * ptr)
    {
        ptr_.reset(ptr);
    }

    void set(boost::shared_ptr<BaseType> ptr)
    {
        ptr_ = ptr;
    }

    void set(const BaseType& x)
    {
        ptr_.reset(new BaseType(x));
    }

private:
    boost::shared_ptr<BaseType> ptr_;
};

#endif /* VIRTUAL_CLASS_H */

