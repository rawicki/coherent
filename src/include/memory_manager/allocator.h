/*
 * (C) Copyright 2010 Xilexio
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

#ifndef MEMORY_ALLOCATOR_H
#define	MEMORY_ALLOCATOR_H

#include <cstddef>
#include <bits/allocator.h>
#include <sys/mman.h>
#include <memory_manager/session.h>
#include <memory_manager/sub_session.h>
#include <debug/asserts.h>

namespace coherent
{
namespace memory_manager
{

typedef boost::unique_lock<boost::recursive_mutex> scoped_lock;

class out_of_session_ram_memory: public std::exception
{

    virtual const char* what() const throw ()
    {
	return "Insufficent ram session memory left for memory allocation.";
    }
};

class out_of_session_total_memory: public std::exception
{

    virtual const char* what() const throw ()
    {
	return "Insufficent total session memory left for memory allocation.";
    }
};

template <class T>
class allocator;

// specialize for void

template <>
class allocator<void>: public std::allocator<void>
{
};

template <class T>
class allocator: public std::allocator<T>
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<typename W>
    struct rebind
    {
	typedef allocator<W> other;
    };

    allocator() throw (): std::allocator<T>()
    {
    }

    allocator(const allocator& orig) throw (): std::allocator<T>(orig)
    {
    }

    template <class U>
    allocator(const allocator<U>& orig) throw (): std::allocator<T>(orig)
    {
    }

    ~allocator() throw ()
    {
    }

    pointer allocate(size_type n, allocator<void>::const_pointer hint = 0)
    {
	if (n == 0)
	    return 0;

	size_t needed_bytes = sizeof (T) * n;

	memory_sub_session* mss = memory_sub_session::current();
	d_assert(mss);
	d_assert(mss->frozen.get() && !*mss->frozen.get());

	scoped_lock sl(mss->mutex);

	memory_session* ms = mss->get_parent();
	d_assert(ms);

	scoped_lock l(ms->mutex);
	d_assert(ms->ram_limit_bytes <= std::allocator<T>::max_size());
	d_assert(ms->total_limit_bytes <= std::allocator<T>::max_size());

	if (ms->ram_allocated_bytes + needed_bytes > ms->ram_limit_bytes)
	    throw out_of_session_ram_memory();

	if (ms->total_allocated_bytes + needed_bytes > ms->total_limit_bytes)
	    throw out_of_session_total_memory();

	pointer res = reinterpret_cast<pointer> (mss->allocate(needed_bytes));

	return res;
    }

    void deallocate(pointer ptr, size_type n)
    {
	if (n == 0 || !ptr)
	    return;

	memory_sub_session* mss = memory_sub_session::current();
	d_assert(mss);

	scoped_lock sl(mss->mutex);

	memory_session* ms = mss->get_parent();
	d_assert(ms);

	scoped_lock l(ms->mutex);

	byte* p = reinterpret_cast<byte*> (ptr);
	size_t bytes = sizeof (T) * n;

	mss->deallocate(p, bytes);
    }

    size_type max_size() const throw ()
    {
	memory_session* ms = memory_session::current();
	d_assert(ms);

	scoped_lock l(ms->mutex);
	d_assert(ms->ram_limit_bytes <= std::allocator<T>::max_size());
	d_assert(ms->total_limit_bytes <= std::allocator<T>::max_size());

	size_type max_bytes;

	if (ms->ram_allocated_bytes <= ms->ram_limit_bytes && ms->total_limit_bytes <= ms->total_limit_bytes)
	{
	    max_bytes = ms->ram_limit_bytes - ms->ram_allocated_bytes;

	    size_type max_total_bytes = ms->total_limit_bytes - ms->total_allocated_bytes;

	    if (max_bytes > max_total_bytes)
		max_bytes = max_total_bytes;
	}
	else
	{
	    max_bytes = 0;
	}

	return max_bytes;
    }
};

template <class T1, class T2>
bool operator ==(const allocator<T1>&, const allocator<T2>&) throw ()
{
    return true;
}

template <class T1, class T2>
bool operator !=(const allocator<T1>&, const allocator<T2>&) throw ()
{
    return false;
}

template <typename T>
allocator<T>& allocator_instance()
{
    static allocator<T> instance;
    return instance;
}

template <typename T>
T* allocate(size_t size)
{
    return allocator<T>().allocate(size);
}

template <typename T>
void deallocate(T* p, size_t size)
{
    return allocator<T>().deallocate(p, size);
}

}
}

#endif
