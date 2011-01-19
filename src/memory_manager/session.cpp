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

#include <cassert>
#include <sys/mman.h>
#include <cstdio> // TODO delete
#include <memory_manager/manager.h>
#include <memory_manager/session.h>
#include <memory_manager/sub_session.h>
#include <memory_manager/pthread_wrapper.h>
#include <debug/asserts.h>
#include "thread.h"

namespace coherent
{
	namespace memory_manager
	{

		memory_session::memory_session(bool autostart) : allocated_bytes(0), default_sub_session(new memory_sub_session(this, false))
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

			assert(!pthread_mutex_destroy(&active_threads_mutex));
			assert(!pthread_rwlock_destroy(&limit_lock));
			assert(!pthread_rwlock_destroy(&alloc_lock));
			assert(!pthread_rwlock_destroy(&sub_sessions_lock));

			delete default_sub_session;

			memory_manager::instance->free_bytes(limit_bytes);

			// TODO możliwa asercja na wyczyszczenie pamięci
		}

		memory_session*
		memory_session::current()
		{
			return memory_sub_session::current()->get_parent();
		}

		void
		memory_session::begin()
		{
			memory_thread_init_if_needed();

			throw 5;
			//r_assert(false, "nyan");

			activate();

			default_sub_session->begin();

			scoped_mutex am(&active_threads_mutex);
			++active_threads_count;
		}

		void
		memory_session::end()
		{
			deactivate();

			scoped_mutex am(&active_threads_mutex);
			if (active_threads_count > 0)
				--active_threads_count;
		}

		void
		memory_session::stop()
		{
			deactivate();

			scoped_rwlock_read ss(&sub_sessions_lock);

			for (std::set<memory_sub_session*>::const_iterator i = sub_sessions.begin(); i != sub_sessions.end(); ++i) {
				(*i)->stop();
			}

			scoped_mutex am(&active_threads_mutex);
			assert(active_threads_count > 0);
			--active_threads_count;
		}

		void
		memory_session::resume()
		{
			{
				scoped_mutex am(&active_threads_mutex);
				++active_threads_count;
			}

			activate();
			default_sub_session->resume();
		}

		void
		memory_session::set_default_current()
		{
			default_sub_session->set_current();
		}

		size_t
		memory_session::get_limit_bytes() const
		{
			scoped_rwlock_read ll(&limit_lock);
			return limit_bytes;
		}

		void
		memory_session::set_limit_bytes(size_t bytes)
		{
			scoped_rwlock_write ll(&limit_lock);
			limit_bytes = bytes;
		}

		size_t
		memory_session::get_allocated_bytes() const
		{
			scoped_rwlock_read al(&alloc_lock);
			return allocated_bytes;
		}

		void
		memory_session::internal_init(bool autostart)
		{
			memory_manager::instance->reserve_bytes(limit_bytes);

			sub_sessions.insert(default_sub_session);

			assert(!pthread_mutex_init(&active_threads_mutex, 0));
			assert(!pthread_rwlock_init(&limit_lock, 0));
			assert(!pthread_rwlock_init(&alloc_lock, 0));
			assert(!pthread_rwlock_init(&sub_sessions_lock, 0));

			if (autostart)
				begin();
		}

		void
		memory_session::activate()
		{
			tls_content* tls_content = tls();

			if (tls_content->current_sub_session && tls_content->current_sub_session->get_parent() != this)
				tls_content->current_sub_session->get_parent()->stop();
			// TODO może dać asserta tylko?

			tls_content->current_sub_session = default_sub_session;
		}

		void
		memory_session::deactivate()
		{
			tls_content* tls_content = tls();

			if (tls_content->current_sub_session && tls_content->current_sub_session->get_parent() == this)
				tls_content->current_sub_session = 0;
		}

	}
}