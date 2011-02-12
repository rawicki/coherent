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

#include <sys/mman.h>
#include <memory_manager/manager.h>
#include <memory_manager/session.h>
#include <memory_manager/sub_session.h>
#include <debug/asserts.h>

namespace coherent
{
namespace memory_manager
{

typedef boost::shared_lock<boost::shared_mutex> scoped_read_lock;
typedef boost::unique_lock<boost::shared_mutex> scoped_write_lock;
typedef boost::unique_lock<boost::mutex> scoped_lock;

void dummy_function(memory_sub_session*) {}
boost::thread_specific_ptr<memory_sub_session> memory_session::current_sub_session(dummy_function);

memory_session::memory_session(bool autostart): allocated_bytes(0), default_sub_session(new memory_sub_session(this, false))
{
    limit_bytes = memory_manager::instance->get_default_session_limit_bytes();

    internal_init(autostart);
}

memory_session::memory_session(size_t starting_limit_bytes, bool autostart) : limit_bytes(starting_limit_bytes), allocated_bytes(0), default_sub_session(new memory_sub_session(this, false))
{
    internal_init(autostart);
}

memory_session::~memory_session()
{
    end();

    delete default_sub_session;

    memory_manager::instance->free_bytes(limit_bytes);

    // TODO triggerable assert if there is non-released memory left
}

memory_session* memory_session::current()
{
    if (current_sub_session.get() == 0)
	return 0;
    
    return current_sub_session->get_parent();
}

void memory_session::begin()
{
    activate();

    default_sub_session->begin();

    scoped_lock am(active_threads_mutex);
    ++active_threads_count;
}

void memory_session::end()
{
    deactivate();

    scoped_lock am(active_threads_mutex);
    if (active_threads_count > 0)
	--active_threads_count;
}

void memory_session::stop()
{
    deactivate();

    scoped_read_lock ss(sub_sessions_lock);

    for (std::set<memory_sub_session*>::const_iterator i = sub_sessions.begin(); i != sub_sessions.end(); ++i)
    {
	(*i)->stop();
    }

    scoped_lock am(active_threads_mutex);
    d_assert(active_threads_count > 0);
    --active_threads_count;
}

void memory_session::resume()
{
    {
	scoped_lock am(active_threads_mutex);
	++active_threads_count;
    }

    activate();
    default_sub_session->resume();
}

void memory_session::set_default_current()
{
    default_sub_session->set_current();
}

size_t memory_session::get_limit_bytes() const
{
    scoped_read_lock ll(limit_lock);
    return limit_bytes;
}

void memory_session::set_limit_bytes(size_t bytes)
{
    scoped_write_lock ll(limit_lock);
    limit_bytes = bytes;
}

size_t memory_session::get_allocated_bytes() const
{
    scoped_read_lock al(alloc_lock);
    return allocated_bytes;
}

void memory_session::internal_init(bool autostart)
{
    memory_manager::instance->reserve_bytes(limit_bytes);

    sub_sessions.insert(default_sub_session);

    if (autostart)
	begin();
}

void memory_session::activate()
{   
    if (current_sub_session.get() && current_sub_session->get_parent() != this)
	current_sub_session->get_parent()->stop();
    // TODO maybe only assert here

    default_sub_session->set_current();
}

void memory_session::deactivate()
{
    if (current_sub_session.get() && current_sub_session->get_parent() == this)
	current_sub_session.reset();
}

}
}