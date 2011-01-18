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

#ifndef MISC_POINTER_POLICY_H
#define MISC_POINTER_POLICY_H

#include <memory>
#include <boost/shared_ptr.hpp>

namespace coherent {
namespace misc {


template <typename T, typename Ptr = T*>
struct std_ptr_policy
{
    typedef Ptr pointer_type;

    std_ptr_policy() : ptr_(NULL)
    {
    }
    std_ptr_policy(Ptr ptr) : ptr_(NULL)
    {
        set(ptr);
    }
    std_ptr_policy(const T& x) : ptr_(NULL)
    {
        set(x);
    }

    bool is_null() const
    {
        return ptr_==NULL;
    }

    const Ptr get_ptr() const
    {
        return ptr_;
    }
    Ptr get_ptr()
    {
        return ptr_;
    }
    const T& get() const
    {
        return *ptr_;
    }
    T& get()
    {
        return *ptr_;
    }

    void set(Ptr ptr)
    {
        if (ptr_)
            delete ptr_;
        ptr_ = ptr;
    }
    void set(const T& x)
    {
        if (ptr_)
            delete ptr_;
        ptr_ = new T(x);
    }
    void reset()
    {
        if (ptr_)
            delete ptr_;
        ptr_ = NULL;
    }

protected:
    Ptr ptr_;
};


template <typename T, typename Ptr = std::auto_ptr<T> >
struct auto_ptr_policy
{
    typedef Ptr pointer_type;

    auto_ptr_policy()
    {
    }
    auto_ptr_policy(Ptr ptr)
    {
        set(ptr);
    }
    auto_ptr_policy(const T& x)
    {
        set(x);
    }

    bool is_null() const
    {
        return ptr_.get()==NULL;
    }

    const Ptr get_ptr() const
    {
        return ptr_;
    }
    Ptr get_ptr()
    {
        return ptr_;
    }
    const T& get() const
    {
        return *ptr_;
    }
    T& get()
    {
        return *ptr_;
    }

    void set(Ptr ptr)
    {
        ptr_ = ptr;
    }
    void set(T * ptr)
    {
        ptr_.reset(ptr);
    }
    void set(const T& x)
    {
        ptr_.reset(new T(x));
    }
    void reset()
    {
        ptr_.reset();
    }

protected:
    Ptr ptr_;
};


template <typename T, typename Ptr = boost::shared_ptr<T> >
struct shared_ptr_policy
{
    typedef Ptr pointer_type;

    shared_ptr_policy()
    {
    }
    shared_ptr_policy(Ptr ptr)
    {
        set(ptr);
    }
    shared_ptr_policy(const T& x)
    {
        set(x);
    }

    bool is_null() const
    {
        return ptr_.get()==NULL;
    }

    const Ptr get_ptr() const
    {
        return ptr_;
    }
    Ptr get_ptr()
    {
        return ptr_;
    }
    const T& get() const
    {
        return *ptr_;
    }
    T& get()
    {
        return *ptr_;
    }

    void set(Ptr ptr)
    {
        ptr_ = ptr;
    }
    void set(T * ptr)
    {
        ptr_.reset(ptr);
    }
    void set(const T& x)
    {
        ptr_.reset(new T(x));
    }
    void reset()
    {
        ptr_.reset();
    }

protected:
    Ptr ptr_;
};


} // namespace misc
} // namespace coherent

#endif /* MISC_POINTER_POLICY_H */
