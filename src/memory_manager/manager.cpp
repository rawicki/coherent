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

#include <algorithm>
#include <unistd.h>
#include <memory_manager/manager.h>
#include <debug/asserts.h>
#include <config/config.h>

namespace coherent
{
namespace memory_manager
{

typedef boost::unique_lock<boost::mutex> scoped_lock;

const char* out_of_ram_memory::what() const throw ()
{
    return "Insufficent managed ram memory left for memory allocation.";
}

const char* out_of_total_memory::what() const throw ()
{
    return "Insufficent managed total memory left for memory allocation.";
}

memory_manager* memory_manager::instance = 0;

memory_manager::~memory_manager()
{
}

void memory_manager::reserve_ram_bytes(boost::condition_variable* unfreeze_condition, size_t bytes)
{
    scoped_lock l(mutex);

    if (bytes > ram_limit_bytes)
	throw out_of_ram_memory();

    unfreeze_queue.push_back(unfreeze_condition);

    while (unfreeze_queue.front() != unfreeze_condition || ram_reserved_bytes + bytes > ram_limit_bytes)
    {
	unfreeze_condition->wait(l);
    }

    unfreeze_queue.pop_front();

    ram_reserved_bytes += bytes;
}

bool memory_manager::try_reserve_ram_bytes(size_t bytes)
{
    scoped_lock l(mutex);

    if (bytes > ram_limit_bytes)
	throw out_of_ram_memory();

    if (ram_reserved_bytes + bytes > ram_limit_bytes)
	return false;

    ram_reserved_bytes += bytes;

    return true;
}

bool memory_manager::timed_reserve_ram_bytes(boost::condition_variable* unfreeze_condition, size_t bytes, const boost::system_time& abs_time)
{
    scoped_lock l(mutex);

    if (bytes > ram_limit_bytes)
	throw out_of_ram_memory();

    unfreeze_queue.push_back(unfreeze_condition);

    while (unfreeze_queue.front() != unfreeze_condition || ram_reserved_bytes + bytes > ram_limit_bytes)
    {
	if (!unfreeze_condition->timed_wait(l, abs_time))
	{
	    unfreeze_queue.erase(std::find(unfreeze_queue.begin(), unfreeze_queue.end(), unfreeze_condition));
	    return false;
	}
    }

    unfreeze_queue.pop_front();

    ram_reserved_bytes += bytes;

    return true;
}

void memory_manager::free_ram_bytes(size_t bytes)
{
    scoped_lock l(mutex);

    ram_reserved_bytes -= bytes;

    if (unfreeze_queue.size() != 0)
    {
	unfreeze_queue.front()->notify_one();
    }
}

void memory_manager::reserve_total_bytes(boost::condition_variable* unfreeze_condition, size_t bytes)
{
    scoped_lock l(mutex);

    if (bytes > total_limit_bytes)
	throw out_of_total_memory();

    unfreeze_queue.push_back(unfreeze_condition);

    while (unfreeze_queue.front() != unfreeze_condition || total_reserved_bytes + bytes > total_limit_bytes)
    {
	unfreeze_condition->wait(l);
    }

    unfreeze_queue.pop_front();

    total_reserved_bytes += bytes;
}

bool memory_manager::try_reserve_total_bytes(size_t bytes)
{
    scoped_lock l(mutex);

    if (bytes > total_limit_bytes)
	throw out_of_total_memory();

    if (total_reserved_bytes + bytes > total_limit_bytes)
	return false;

    total_reserved_bytes += bytes;

    return true;
}

bool memory_manager::timed_reserve_total_bytes(boost::condition_variable* unfreeze_condition, size_t bytes, const boost::system_time& abs_time)
{
    scoped_lock l(mutex);

    if (bytes > total_limit_bytes)
	throw out_of_total_memory();

    unfreeze_queue.push_back(unfreeze_condition);

    while (unfreeze_queue.front() != unfreeze_condition || total_reserved_bytes + bytes > total_limit_bytes)
    {
	if (!unfreeze_condition->timed_wait(l, abs_time))
	{
	    unfreeze_queue.erase(std::find(unfreeze_queue.begin(), unfreeze_queue.end(), unfreeze_condition));
	    return false;
	}
    }

    unfreeze_queue.pop_front();

    total_reserved_bytes += bytes;

    return true;
}

void memory_manager::free_total_bytes(size_t bytes)
{
    scoped_lock l(mutex);

    total_reserved_bytes -= bytes;

    if (unfreeze_queue.size() != 0)
    {
	unfreeze_queue.front()->notify_one();
    }
}

void memory_manager::init(const config::global_config& conf)
{
    d_assert(!instance);

    instance = new memory_manager(conf);
}

memory_manager::memory_manager(const config::global_config& conf):
ram_reserved_bytes(0), total_reserved_bytes(0),
ram_limit_bytes(conf.memory_manager.ram_limit_bytes),
total_limit_bytes(conf.memory_manager.total_limit_bytes),
default_session_ram_limit_bytes(conf.memory_manager.default_session_ram_limit_bytes),
default_session_total_limit_bytes(conf.memory_manager.default_session_total_limit_bytes),
max_small_alloc_pages_1024(conf.memory_manager.max_small_alloc_pages_1024),
single_small_alloc_pages(conf.memory_manager.single_small_alloc_pages)
{
    page_size = sysconf(_SC_PAGESIZE);
}

}
}
