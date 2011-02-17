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

#ifndef MEMORY_SUB_SESSION_H
#define MEMORY_SUB_SESSION_H

#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

namespace coherent
{
namespace memory_manager
{

typedef char byte;
class memory_session;

class memory_sub_session: private boost::noncopyable
{
public:
    memory_sub_session(bool autostart = true);
    memory_sub_session(memory_session* session, bool autostart = true);
    ~memory_sub_session();

    static memory_sub_session* current();

    // on initial begin, sub session begins in frozen state if it or its parent session is frozen on all other threads, otherwise unfrozen
    // you can ensure session is frozen/unfrozen by calling freeze/unfreeze explicitly
    void begin();
    void end();
    void freeze();
    bool unfreeze();
    bool is_frozen() const;
    void set_current();

    size_t get_allocated_bytes() const;
    memory_session* get_parent() const;

private:
    void internal_init(bool autostart);

    // marks and unmarks session as being used in current context in this thread
    void activate();
    void deactivate();

    // TODO write comment what locks are needed
    byte* allocate(size_t needed_bytes);
    void deallocate(byte* p, size_t bytes);
    void add_alloc(byte* p, size_t bytes);
    void remove_alloc(byte* p);
    std::pair<size_t, byte*> smallest_sufficent_free_small_chunk(size_t bytes);
    void add_small_alloc(byte* p, size_t bytes);
    void remove_small_alloc(byte* p);
    bool is_small_alloc(byte* p) const;
    void add_free_small_chunk(byte* p, size_t bytes);
    void remove_free_small_chunk(byte* p);
    std::pair<byte*, size_t> free_small_chunk_alloc(byte* p) const;
    std::pair<byte*, size_t> add_merge_remove_free_small_chunk(byte* p, size_t bytes);

    template <typename T>
    friend class allocator;

    memory_session * const parent;

    boost::thread_specific_ptr<bool> frozen;

    // variables accessed with lock on
    int unfrozen_threads_count;

    size_t allocated_bytes;
    std::map<byte*, size_t> allocs;
    std::set<std::pair<byte*, size_t> > small_allocs;
    std::map<byte*, size_t> free_small_chunks;
    std::set<std::pair<size_t, byte*> > free_small_chunks_inv;

    mutable boost::recursive_mutex mutex;
};

}
}

#endif
