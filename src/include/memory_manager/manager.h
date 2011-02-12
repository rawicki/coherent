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
#include <stdint.h>

namespace coherent
{

namespace config
{
class global_config;
}

namespace memory_manager
{

class out_of_total_memory : public std::exception
{
    virtual const char* what() const throw();
};

class memory_manager : private boost::noncopyable
{
public:
    ~memory_manager();

    size_t get_page_size() const throw()
    {
	return page_size;
    }

    size_t get_default_session_limit_bytes() const throw()
    {
	return default_session_limit_bytes;
    }

    void reserve_bytes(size_t bytes);
    void free_bytes(size_t bytes);

    /// Creates instance of memory manager. Must be invoked before any memory manager operations.
    static void init(const config::global_config& conf);
    static memory_manager* instance;

private:
    memory_manager(const config::global_config& conf);

    uint64_t reserved_bytes;
    mutable boost::mutex reserved_bytes_mutex;
    uint64_t limit_bytes;
    size_t page_size;
    size_t default_session_limit_bytes;
};

}
}

#endif
