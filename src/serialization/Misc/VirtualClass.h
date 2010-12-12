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
#include "Misc/TypeList.h"
#include "Misc/TypeTree.h"



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


template <typename F, typename CurrentType, typename LeftNode, typename RightNode>
struct Fold<F, TreeNode<CurrentType, LeftNode, RightNode> >
{
    void operator() (F& f) const
    {
        int ret = f.template processNode<CurrentType>();
        if (ret==0) {
            return;
        }
        if (ret<0) {
            Fold<F, LeftNode>() (f);
            return;
        }
        Fold<F, RightNode>() (f); //ret>0
    }
};

template <typename F>
struct Fold<F, TreeLeaf>
{
    void operator() (F&) const
    {
        throw "Tree Leaf!";
    }
};


namespace checker_detail
{

template <typename Node>
struct TreeChecker;

template <>
struct TreeChecker<TreeLeaf>
{
    static const bool ok = true;
};

template <typename Current>
struct TreeChecker<TreeNode<Current, TreeLeaf, TreeLeaf> >
{
    static const int curr_ = Current::TAG;
    static const int min_ = curr_;
    static const int max_ = curr_;
    static const bool ok = true;
};

template <typename Current, typename LType, typename LLeftNode, typename LRightNode>
struct TreeChecker<TreeNode<Current, TreeNode<LType, LLeftNode, LRightNode>, TreeLeaf> >
{
    typedef TreeChecker<TreeNode<LType, LLeftNode, LRightNode> > lchecker;
    static const int curr_ = Current::TAG;
    static const int min_ = (curr_<lchecker::min_) ? curr_ : (lchecker::min_);
    static const int max_ = (curr_>lchecker::max_) ? curr_ : (lchecker::max_);
    static const bool ok = lchecker::ok && (lchecker::max_<curr_);
};

template <typename Current, typename RType, typename RLeftNode, typename RRightNode>
struct TreeChecker<TreeNode<Current, TreeLeaf, TreeNode<RType, RLeftNode, RRightNode> > >
{
    typedef TreeChecker<TreeNode<RType, RLeftNode, RRightNode> > rchecker;
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
struct TreeChecker<TreeNode<Current, TreeNode<LType, LLeftNode, LRightNode>, TreeNode<RType, RLeftNode, RRightNode> > >
{
    typedef TreeChecker<TreeNode<LType, LLeftNode, LRightNode> > lchecker;
    typedef TreeChecker<TreeNode<RType, RLeftNode, RRightNode> > rchecker;
    static const int curr_ = Current::TAG;
    static const int child_min_ = (lchecker::min_<rchecker::min_) ? (lchecker::min_) : (rchecker::min_);
    static const int child_max_ = (lchecker::max_>rchecker::max_) ? (lchecker::max_) : (rchecker::max_);
    static const int min_ = (curr_<child_min_) ? curr_ : child_min_;
    static const int max_ = (curr_>child_max_) ? curr_ : child_max_;
    static const bool ok = (lchecker::ok) && (rchecker::ok) && (curr_>lchecker::max_) && (curr_<rchecker::min_);
};

} //namespace checker_detail


template <typename Tree>
inline void checkTree()
{
    assert(checker_detail::TreeChecker<Tree>::ok);
}


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
            if ((TagType)Type::TAG == tag_) {
                enc_((TagType)Type::TAG);
                enc_(static_cast<const Type&>(*ptr_));
                return true;
            }
            return false;
        }
        template <typename Type>
        int processNode() {
            if ((TagType)Type::TAG == tag_) {
                enc_((TagType)Type::TAG);
                enc_(static_cast<const Type&>(*ptr_));
                return 0;
            }
            if (tag_ < (TagType)Type::TAG) {
                return -1;
            }
            return 1;
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
            if ((TagType)Type::TAG == tag_) {
                ptr_.reset(new Type());
                dec_(static_cast<Type&>(*ptr_));
                return true;
            }
            return false;
        }
        template <typename Type>
        int processNode() {
            if ((TagType)Type::TAG == tag_) {
                ptr_.reset(new Type());
                dec_(static_cast<Type&>(*ptr_));
                return 0;
            }
            if (tag_ < (TagType)Type::TAG)
                return -1;
            return 1;
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


    //constuctors
    Virtual()
    {
    }

    Virtual(boost::shared_ptr<BaseType> ptr) : ptr_(ptr)
    {
    }

    const BaseType * get_ptr() const
    {
        return ptr_.get();
    }

    boost::shared_ptr<BaseType> get_sptr() const
    {
        return ptr_;
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

