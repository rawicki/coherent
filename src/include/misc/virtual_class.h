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

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <inttypes.h>

#include <debug/asserts.h>
#include <misc/pointer_policy.h>
#include <misc/type_list.h>
#include <misc/type_tree.h>


namespace coherent {
namespace misc {


template <typename F, typename TypeList>
struct fold;

template <typename F, typename CurrentType, typename TypeList>
struct fold<F, list_elem<CurrentType, TypeList> >
{
    void operator() (F& f) const
    {
        if (f.template process<CurrentType>())
            return;
        fold<F, TypeList>() (f);
    }
};

template <typename F>
struct fold<F, list_head>
{
    void operator() (F&) const
    {
        throw std::runtime_error("End of List!");
    }
};


template <typename F, typename CurrentType, typename LeftNode, typename RightNode>
struct fold<F, tree_node<CurrentType, LeftNode, RightNode> >
{
    void operator() (F& f) const
    {
        int ret = f.template process_node<CurrentType>();
        if (ret==0) {
            return;
        }
        if (ret<0) {
            fold<F, LeftNode>() (f);
            return;
        }
        fold<F, RightNode>() (f); //ret>0
    }
};

template <typename F>
struct fold<F, tree_leaf>
{
    void operator() (F&) const
    {
        throw std::runtime_error("Tree Leaf!");
    }
};


template <typename Factory>
struct object_factory;


template <typename F, typename Factory>
struct fold<F, object_factory<Factory> >
{
    void operator() (F & f) const
    {
        typename Factory::template Factory<F>() (f);
    }
};

namespace checker_detail
{

template <typename Node>
struct tree_checker;

template <>
struct tree_checker<tree_leaf>
{
    static const bool ok = true;
};

template <typename Current>
struct tree_checker<tree_node<Current, tree_leaf, tree_leaf> >
{
    static const int curr_ = Current::TAG;
    static const int min_ = curr_;
    static const int max_ = curr_;
    static const bool ok = true;
};

template <typename Current, typename LType, typename LLeftNode, typename LRightNode>
struct tree_checker<tree_node<Current, tree_node<LType, LLeftNode, LRightNode>, tree_leaf> >
{
    typedef tree_checker<tree_node<LType, LLeftNode, LRightNode> > lchecker;
    static const int curr_ = Current::TAG;
    static const int min_ = (curr_<lchecker::min_) ? curr_ : (lchecker::min_);
    static const int max_ = (curr_>lchecker::max_) ? curr_ : (lchecker::max_);
    static const bool ok = lchecker::ok && (lchecker::max_<curr_);
};

template <typename Current, typename RType, typename RLeftNode, typename RRightNode>
struct tree_checker<tree_node<Current, tree_leaf, tree_node<RType, RLeftNode, RRightNode> > >
{
    typedef tree_checker<tree_node<RType, RLeftNode, RRightNode> > rchecker;
    static const int curr_ = Current::TAG;
    static const int min_ = (curr_<rchecker::min_) ? curr_ : (rchecker::min_);
    static const int max_ = (curr_>rchecker::max_) ? curr_ : (rchecker::max_);
    static const bool ok = (rchecker::ok) && (rchecker::min_ > curr_);
};

template <
    typename Current,
    typename LType, typename LLeftNode, typename LRightNode,
    typename RType, typename RLeftNode, typename RRightNode
>
struct tree_checker<tree_node<Current, tree_node<LType, LLeftNode, LRightNode>, tree_node<RType, RLeftNode, RRightNode> > >
{
    typedef tree_checker<tree_node<LType, LLeftNode, LRightNode> > lchecker;
    typedef tree_checker<tree_node<RType, RLeftNode, RRightNode> > rchecker;
    static const int curr_ = Current::TAG;
    static const int child_min_ = (lchecker::min_<rchecker::min_) ? (lchecker::min_) : (rchecker::min_);
    static const int child_max_ = (lchecker::max_>rchecker::max_) ? (lchecker::max_) : (rchecker::max_);
    static const int min_ = (curr_<child_min_) ? curr_ : child_min_;
    static const int max_ = (curr_>child_max_) ? curr_ : child_max_;
    static const bool ok = (lchecker::ok) && (rchecker::ok) && (curr_>lchecker::max_) && (curr_<rchecker::min_);
};

} //namespace checker_detail


template <typename Tree>
inline void check_tree()
{
    r_assert(checker_detail::tree_checker<Tree>::ok, "check_tree error");
}


template <typename BaseType, typename TypeList, typename PointerPolicy = shared_ptr_policy<BaseType> >
struct Virtual : public PointerPolicy
{
    typedef Virtual<BaseType, TypeList, PointerPolicy>  self;
    typedef typename BaseType::tag_type                 tag_type;
    typedef typename PointerPolicy::pointer_type        pointer_type;
private:
    template <typename Encoder>
    struct tagged_encoder
    {
        tagged_encoder(Encoder& enc, const self& ptr) : enc_(enc), ptr_(ptr), tag_(ptr_.get().get_tag())
        {
        }
        template <typename Type>
        bool process() {
            if ((tag_type)Type::TAG == tag_) {
                enc_((tag_type)Type::TAG);
                enc_(static_cast<const Type&>(ptr_.get()));
                return true;
            }
            return false;
        }
        template <typename Type>
        int process_node() {
            if ((tag_type)Type::TAG == tag_) {
                enc_((tag_type)Type::TAG);
                enc_(static_cast<const Type&>(ptr_.get()));
                return 0;
            }
            if (tag_ < (tag_type)Type::TAG) {
                return -1;
            }
            return 1;
        }

        template <typename Type>
        void process_unchecked() {
            enc_((tag_type)Type::TAG);
            enc_(static_cast<const Type&>(ptr_.get()));
        }
        template <typename Type>
        void process_checked() {
            r_assert( (tag_type)Type::TAG == tag_, "tag mismatch");
            process_unchecked<Type>();
        }

        Encoder& enc_;
        const self& ptr_;
        tag_type tag_;
    };
    template <typename Decoder>
    struct tagged_decoder
    {
        tagged_decoder(Decoder& dec, self& ptr, tag_type tag) : dec_(dec), ptr_(ptr), tag_(tag)
        {
        }
        template <typename Type>
        bool process() {
            if ((tag_type)Type::TAG == tag_) {
                ptr_.set(new Type());
                dec_(static_cast<Type&>(ptr_.get()));
                return true;
            }
            return false;
        }
        template <typename Type>
        int process_node() {
            if ((tag_type)Type::TAG == tag_) {
                ptr_.set(new Type());
                dec_(static_cast<Type&>(ptr_.get()));
                return 0;
            }
            if (tag_ < (tag_type)Type::TAG)
                return -1;
            return 1;
        }

        template <typename Type>
        void process_unchecked() {
            ptr_.set(new Type());
            dec_(static_cast<Type&>(ptr_.get()));
        }
        template <typename Type>
        void process_checked() {
            r_assert( (tag_type)Type::TAG == tag_, "tag mismatch");
            process_unchecked<Type>();
        }

        Decoder& dec_;
        self& ptr_;
        tag_type tag_;
    };
public:
    template <typename Encoder>
    void encode(Encoder& enc) const
    {
        if (PointerPolicy::is_null()) {
            enc(tag_type());
            return;
        }
        tagged_encoder<Encoder> te(enc, *this);
        fold<tagged_encoder<Encoder>, TypeList>()(te);
    }

    template <typename Decoder>
    void decode(Decoder& dec)
    {
        tag_type tag;
        dec(tag);
        if (tag==tag_type()) {
            PointerPolicy::reset();
            return;
        }
        tagged_decoder<Decoder> td(dec, *this, tag);
        fold<tagged_decoder<Decoder>, TypeList>()(td);
    }

    //constuctors
    Virtual()
    {
    }

    Virtual(pointer_type ptr) : PointerPolicy(ptr)
    {
    }

    Virtual(const BaseType& x) : PointerPolicy(x)
    {
    }

    struct helper
    {
        template <typename F>
        helper(F & f, pointer_type& ptr)
        {
            self vc;
            f(vc);
            ptr = vc.get_ptr();
        }
        template <typename F>
        helper(F & f, const pointer_type& ptr)
        {
            f(self(ptr));
        }
    };

    template <typename F>
    void for_each(F & f)
    {
        decode<F>(f);
    }
    template <typename F>
    void for_each(F & f) const
    {
        encode<F>(f);
    }

};


} // namespace misc
} // namespace coherent

#endif /* VIRTUAL_CLASS_H */

