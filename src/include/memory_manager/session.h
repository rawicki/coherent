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

#include "sub_session.h"


#include <cstddef>
#include <boost/noncopyable.hpp>
#include <map>
#include <set>
#include <utility>
#include <pthread.h>

namespace coherent
{
namespace memory_manager
{

typedef char byte;
class memory_sub_session;

class memory_session : private boost::noncopyable
{
public:
    memory_session(bool autostart = true);
    memory_session(size_t starting_limit_bytes, bool autostart = true);
    ~memory_session();

    static memory_session* current();

    void begin();
    void end();
    void stop();
    void resume();
    void set_default_current();

    size_t get_limit_bytes() const;
    void set_limit_bytes(size_t bytes);
    size_t get_allocated_bytes() const;

private:
    void internal_init(bool autostart);
    void activate();
    void deactivate();

    template <typename T>
    friend class allocator;

    friend class memory_sub_session;

    size_t active_threads_count;
    mutable pthread_mutex_t active_threads_mutex;

    size_t limit_bytes;
    mutable pthread_rwlock_t limit_lock;

    size_t allocated_bytes;
    std::map<byte*, memory_sub_session*> allocs;
    mutable pthread_rwlock_t alloc_lock;

    memory_sub_session* default_sub_session;

    std::set<memory_sub_session*> sub_sessions;
    mutable pthread_rwlock_t sub_sessions_lock;
};

}
}

#endif