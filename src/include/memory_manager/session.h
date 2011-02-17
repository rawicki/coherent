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
#include <map>
#include <set>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <stdint.h>

namespace coherent
{
namespace memory_manager
{

typedef char byte;
class memory_sub_session;

class memory_session : private boost::noncopyable
{
public:
    // constructs memory session and if autostart true, invokes begin()
    memory_session(bool autostart = true);
    memory_session(size_t starting_ram_limit_bytes, size_t starting_total_limit_bytes, bool autostart = true);
    // destroys memory session and invokes end() if it wasn't called yet
    ~memory_session();

    static memory_session* current();

    // begins usage of this session in this thread, nothing can be allocated before it is called
    // on initial begin, session begins in frozen state if it is frozen on all other threads, otherwise unfrozen
    // you can ensure session is frozen/unfrozen by calling freeze/unfreeze explicitly
    void begin();
    // ends usage of this session in this thread, nothing can be allocated after it is called
    void end();
    // freezes ram memory used by session for this thread (if it isn't used by other threads marks it as moveable to swap, frozen memory must not be used)
    void freeze();
    // variants of unfreezing session. if resources are not available, session is appended to queue and waits for them.
    void unfreeze();    
    bool try_unfreeze();
    bool timed_unfreeze(const boost::system_time& abs_time);
    // returns whether session is frozen in this thread
    bool is_frozen() const;

    // set default sub session as current
    void set_default_current();

    size_t get_ram_limit_bytes() const;
    void set_ram_limit_bytes(size_t bytes);
    size_t get_ram_allocated_bytes() const;

    size_t get_total_limit_bytes() const;
    void set_total_limit_bytes(size_t bytes);
    size_t get_total_allocated_bytes() const;

private:
    void internal_init(bool autostart);

    template <typename T>
    friend class allocator;

    friend class memory_sub_session;

    static boost::thread_specific_ptr<memory_sub_session> current_sub_session;

    boost::thread_specific_ptr<bool> frozen;
    mutable boost::condition_variable unfreeze_condition;

    // variables accessed with lock on
    int unfrozen_threads_count;

    size_t ram_limit_bytes;
    size_t total_limit_bytes;

    size_t ram_allocated_bytes;
    size_t total_allocated_bytes;
    std::map<byte*, memory_sub_session*> allocs;

    memory_sub_session* default_sub_session;
    std::set<memory_sub_session*> sub_sessions;

    mutable boost::recursive_mutex mutex;    
};

}
}

#endif
