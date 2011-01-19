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
#include <unistd.h>
#include <cstdio> // TODO delete
#include <memory_manager/manager.h>
#include <memory_manager/pthread_wrapper.h>
#include <config/config.h>
#include "thread.h"

namespace coherent
{
	namespace memory_manager
	{

		const char*
		out_of_total_memory::what() const throw ()
		{
			return "Insufficent total memory left for memory reservation.";
		}

		memory_manager* memory_manager::instance = 0;

		memory_manager::~memory_manager()
		{
			tls_clean();
			assert(!pthread_mutex_destroy(&reserved_bytes_mutex));
		}

		void
		memory_manager::reserve_bytes(size_t bytes) throw (out_of_total_memory)
		{
			scoped_mutex rm(&reserved_bytes_mutex);

			if (reserved_bytes + bytes > limit_bytes)
				throw out_of_total_memory();

			reserved_bytes += bytes;
		}

		void
		memory_manager::free_bytes(size_t bytes) throw ()
		{
			scoped_mutex rm(&reserved_bytes_mutex);

			assert(bytes <= reserved_bytes);

			reserved_bytes -= bytes;
		}

		void
		memory_manager::init(const config::global_config& conf)
		{
			assert(!instance);

			instance = new memory_manager(conf);
		}

		memory_manager::memory_manager(const config::global_config& conf) :
		reserved_bytes(0), limit_bytes(conf.memory_manager.initialLimitBytes), default_session_limit_bytes(conf.memory_manager.defaultSessionLimitBytes)
		{
			page_size = sysconf(_SC_PAGESIZE);

			tls_init();
			assert(!pthread_mutex_init(&reserved_bytes_mutex, 0));
		}

	}
}