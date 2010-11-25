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
#include <cassert>
#include <bits/allocator.h>
#include <memory_manager/session.h>
#include <sys/mman.h>
#include <cstdio> // TODO delete
#include <memory_manager/pthread_wrapper.h>

namespace coherent {
namespace memory_manager {

class OutOfSessionMemory: public std::exception {
    virtual const char* what() const throw() {
        return "Insufficent session memory left for memory allocation.";
    }
};

template <class T>
class Allocator;

// specialize for void
template <>
class Allocator<void>: public std::allocator<void> {
};

template <class T>
class Allocator: public std::allocator<T> {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<typename W>
    struct rebind {
        typedef Allocator<W> other;
    };

    Allocator() throw (): std::allocator<T>() {}
    Allocator(const Allocator& orig) throw (): std::allocator<T>(orig) {}
    template <class U>
    Allocator(const Allocator<U>& orig) throw (): std::allocator<T>(orig) {}
    ~Allocator() throw () {}

    pointer allocate(size_type n, Allocator<void>::const_pointer hint = 0) {
        if (n == 0)
            return 0;

        MemorySession* ms = MemorySession::current();
        assert(ms);

        size_t bytes = sizeof(T) * n;

        ScopedRWLockRead ll(&ms->limitLock);
        ScopedRWLockWrite al(&ms->allocLock);

        if (n > max_size_noLock())
            throw OutOfSessionMemory();

        void* p = mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);

        if (p == MAP_FAILED)
            throw std::bad_alloc();

        ms->allocatedBytes += bytes;
        ms->allocations.insert(std::make_pair(p, bytes));

        return static_cast<T*>(p);
    }

    void deallocate(pointer p, size_type n) {
        if (n == 0)
            return;

        MemorySession* ms = MemorySession::current();
        assert(ms);

        size_t bytes = sizeof(T) * n;

        ScopedRWLockWrite al(&ms->allocLock);

        if (munmap(static_cast<void*>(p), bytes))
            printf("UNMAP ERROR\n"); // TODO do zapisu do loga

        ms->allocatedBytes -= bytes;
        ms->allocations.erase(p);
    }

    size_type max_size() const throw() {
        MemorySession* ms = MemorySession::current();
        
        assert(ms);
        
        ScopedRWLockRead ll(&ms->limitLock);
        ScopedRWLockRead al(&ms->allocLock);
        
        assert(ms->limitBytes <= base::max_size());

        if (ms->limitBytes >= ms->allocatedBytes)
            return ms->limitBytes - ms->allocatedBytes;
        else
            return 0;
    }

private:
    size_type max_size_noLock() const throw() {
        MemorySession* ms = MemorySession::current();

        assert(ms);
        assert(ms->limitBytes <= base::max_size());

        if (ms->limitBytes >= ms->allocatedBytes)
            return ms->limitBytes - ms->allocatedBytes;
        else
            return 0;
    }

    typedef std::allocator<T> base;
};

template <class T1, class T2>
bool operator ==(const Allocator<T1>&, const Allocator<T2>&) throw () {
    return true;
}

template <class T1, class T2>
bool operator !=(const Allocator<T1>&, const Allocator<T2>&) throw () {
    return false;
}

template <typename T>
Allocator<T>& allocator() {
    static Allocator<T> instance;
    return instance;
}

template <typename T>
T* allocate(size_t size) {
    return allocator<T>().allocate(size);
}

template <typename T>
void deallocate(T* p, size_t size) {
    return allocator<T>().deallocate(p, size);
}

}
}

#endif