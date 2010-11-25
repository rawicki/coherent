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

#ifndef MEMORY_SESSION_H
#define MEMORY_SESSION_H

#include <cstddef>
#include <boost/noncopyable.hpp>
#include <map>
#include <pthread.h>

namespace coherent {
namespace memory_manager {

class MemorySession: private boost::noncopyable {
public:
    MemorySession();
    MemorySession(size_t startingLimitBytes);
    ~MemorySession();

    static MemorySession* current();

    void begin();
    void end();
    void stop();
    void resume();

    size_t getLimitBytes() const;
    void setLimitBytes(size_t bytes);
    size_t getAllocatedBytes() const;

private:
    void internalInit();
    void activate();
    void deactivate();

    template <typename T>
    friend class Allocator;

    size_t activeThreadsCount;
    mutable pthread_mutex_t activeThreadsMutex;
    size_t limitBytes;
    mutable pthread_rwlock_t limitLock;
    size_t allocatedBytes;
    std::map<void*, size_t> allocations;
    mutable pthread_rwlock_t allocLock;
};

}
}

#endif