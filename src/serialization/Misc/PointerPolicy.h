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


template <typename T, typename Ptr = T*>
struct StdPtrPolicy
{
    typedef Ptr PointerType;

    StdPtrPolicy() : ptr_(NULL)
    {
    }
    StdPtrPolicy(Ptr ptr) : ptr_(NULL)
    {
        set(ptr);
    }
    StdPtrPolicy(const T& x) : ptr_(NULL)
    {
        set(x);
    }

    bool isNull() const
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
struct AutoPtrPolicy
{
    typedef Ptr PointerType;

    AutoPtrPolicy()
    {
    }
    AutoPtrPolicy(Ptr ptr)
    {
        set(ptr);
    }
    AutoPtrPolicy(const T& x)
    {
        set(x);
    }

    bool isNull() const
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
struct SharedPtrPolicy
{
    typedef Ptr PointerType;

    SharedPtrPolicy()
    {
    }
    SharedPtrPolicy(Ptr ptr)
    {
        set(ptr);
    }
    SharedPtrPolicy(const T& x)
    {
        set(x);
    }

    bool isNull() const
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




#endif /* MISC_POINTER_POLICY_H */
