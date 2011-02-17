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

typedef boost::unique_lock<boost::recursive_mutex> scoped_lock;

void dummy_function(memory_sub_session*)
{
}
boost::thread_specific_ptr<memory_sub_session> memory_session::current_sub_session(dummy_function);

memory_session::memory_session(bool autostart):
unfrozen_threads_count(-1), ram_allocated_bytes(0), total_allocated_bytes(0)
{
    ram_limit_bytes = memory_manager::instance->get_default_session_ram_limit_bytes();
    total_limit_bytes = memory_manager::instance->get_default_session_total_limit_bytes();

    internal_init(autostart);
}

memory_session::memory_session(size_t starting_ram_limit_bytes, size_t starting_total_limit_bytes, bool autostart):
unfrozen_threads_count(-1), ram_limit_bytes(starting_ram_limit_bytes),
total_limit_bytes(starting_total_limit_bytes), ram_allocated_bytes(0), total_allocated_bytes(0)
{
    internal_init(autostart);
}

memory_session::~memory_session()
{
    end();

    delete default_sub_session;

    memory_manager::instance->free_total_bytes(total_limit_bytes);
    memory_manager::instance->free_ram_bytes(ram_limit_bytes);

    if (total_allocated_bytes != 0)
	LOG(WARN, "memory leak - not all memory was deallocated while destroying memory session\n");
}

memory_session* memory_session::current()
{
    if (current_sub_session.get() == 0)
	return 0;

    return current_sub_session->get_parent();
}

void memory_session::begin()
{
    if (!frozen.get())
    {
	scoped_lock l(mutex);
	frozen.reset(new bool(unfrozen_threads_count == 0));

	if (!*frozen.get())
	{
	    if (unfrozen_threads_count == -1)
		unfrozen_threads_count = 1;
	    else
		++unfrozen_threads_count;
	}
    }

    default_sub_session->begin();
}

void memory_session::end()
{
    if (memory_session::current_sub_session.get() && memory_session::current_sub_session.get()->get_parent() == this)
	memory_session::current_sub_session.reset();
}

void memory_session::freeze()
{
    d_assert(frozen.get());

    if (*frozen.get())
	return;

    scoped_lock l(mutex);
    --unfrozen_threads_count;
    frozen.reset(new bool(true));

    if (unfrozen_threads_count == 0)
    {
	for (std::set<memory_sub_session*>::const_iterator i = sub_sessions.begin(); i != sub_sessions.end(); ++i)
	{
	    (*i)->freeze();
	}

	memory_manager::instance->free_ram_bytes(ram_limit_bytes);
    }
}

void memory_session::unfreeze()
{
    d_assert(frozen.get());

    if (!*frozen.get())
	return;

    scoped_lock l(mutex);
    if (unfrozen_threads_count == 0)
    {
	memory_manager::instance->reserve_ram_bytes(&unfreeze_condition, ram_limit_bytes);

	++unfrozen_threads_count;
	frozen.reset(new bool(false));

	r_assert(default_sub_session->unfreeze());
    }
}

bool memory_session::try_unfreeze()
{
    d_assert(frozen.get());

    if (!*frozen.get())
	return true;

    scoped_lock l(mutex);
    if (unfrozen_threads_count == 0)
    {
	if (!memory_manager::instance->try_reserve_ram_bytes(ram_limit_bytes))
	    return false;

	++unfrozen_threads_count;
	frozen.reset(new bool(false));

	r_assert(default_sub_session->unfreeze());
    }

    return true;
}

bool memory_session::timed_unfreeze(const boost::system_time& abs_time)
{
    d_assert(frozen.get());

    if (!*frozen.get())
	return true;

    scoped_lock l(mutex);
    if (unfrozen_threads_count == 0)
    {
	if (!memory_manager::instance->timed_reserve_ram_bytes(&unfreeze_condition, ram_limit_bytes, abs_time))
	    return false;

	++unfrozen_threads_count;
	frozen.reset(new bool(false));

	r_assert(default_sub_session->unfreeze());
    }

    return true;
}

bool memory_session::is_frozen() const
{
    return *frozen.get();
}

void memory_session::set_default_current()
{
    default_sub_session->set_current();
}

size_t memory_session::get_ram_limit_bytes() const
{
    scoped_lock l(mutex);
    return ram_limit_bytes;
}

void memory_session::set_ram_limit_bytes(size_t bytes)
{
    scoped_lock l(mutex);
    ram_limit_bytes = bytes;

    // TODO to queue
}

size_t memory_session::get_ram_allocated_bytes() const
{
    scoped_lock l(mutex);
    return ram_allocated_bytes;
}

size_t memory_session::get_total_limit_bytes() const
{
    scoped_lock l(mutex);
    return total_limit_bytes;
}

void memory_session::set_total_limit_bytes(size_t bytes)
{
    scoped_lock l(mutex);
    total_limit_bytes = bytes;

    // TODO to queue
}

size_t memory_session::get_total_allocated_bytes() const
{
    scoped_lock l(mutex);
    return total_allocated_bytes;
}

void memory_session::internal_init(bool autostart)
{
    memory_manager::instance->reserve_total_bytes(&unfreeze_condition, total_limit_bytes);
    memory_manager::instance->reserve_ram_bytes(&unfreeze_condition, ram_limit_bytes);

    default_sub_session = new memory_sub_session(this, false);

    if (autostart)
	begin();
}

}
}
