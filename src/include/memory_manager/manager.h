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

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <exception>
#include <deque>
#include <stdint.h>

namespace coherent
{

namespace config
{
class global_config;
}

namespace memory_manager
{

class out_of_ram_memory: public std::exception
{
    virtual const char* what() const throw ();
};

class out_of_total_memory: public std::exception
{
    virtual const char* what() const throw ();
};

class memory_manager: private boost::noncopyable
{
public:
    ~memory_manager();

    size_t get_page_size() const throw ()
    {
	return page_size;
    }

    uint64_t get_default_session_ram_limit_bytes() const throw ()
    {
	return default_session_ram_limit_bytes;
    }

    uint64_t get_default_session_total_limit_bytes() const throw ()
    {
	return default_session_ram_limit_bytes;
    }

    size_t get_max_small_alloc_bytes() const throw ()
    {
	return max_small_alloc_pages_1024 * page_size / 1024;
    }

    size_t get_single_small_alloc_bytes() const throw ()
    {
	return single_small_alloc_pages * page_size;
    }

    void reserve_ram_bytes(boost::condition_variable* unfreeze_condition, size_t bytes);
    bool try_reserve_ram_bytes(size_t bytes);
    bool timed_reserve_ram_bytes(boost::condition_variable* unfreeze_condition, size_t bytes, const boost::system_time& abs_time);
    void free_ram_bytes(size_t bytes);

    void reserve_total_bytes(boost::condition_variable* unfreeze_condition, size_t bytes);
    bool try_reserve_total_bytes(size_t bytes);
    bool timed_reserve_total_bytes(boost::condition_variable* unfreeze_condition, size_t bytes, const boost::system_time& abs_time);
    void free_total_bytes(size_t bytes);

    /// Creates instance of memory manager. Must be invoked before any memory manager operations.
    static void init(const config::global_config& conf);
    static memory_manager* instance;

private:
    memory_manager(const config::global_config& conf);

    friend class memory_session;

    uint64_t ram_reserved_bytes;
    uint64_t total_reserved_bytes;
    std::deque<boost::condition_variable*> unfreeze_queue;
    mutable boost::mutex mutex;

    uint64_t ram_limit_bytes;
    uint64_t total_limit_bytes;
    uint64_t default_session_ram_limit_bytes;
    uint64_t default_session_total_limit_bytes;
    uint16_t max_small_alloc_pages_1024;
    uint16_t single_small_alloc_pages;
    size_t page_size;
};

}
}

#endif
