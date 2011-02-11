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

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <boost/noncopyable.hpp>

namespace coherent
{
namespace memory_manager
{

class memory_sub_session;

/// thread local storage key
extern pthread_key_t tls_key;

/// Container for thread-specific data used for memory management.

class tls_content : private boost::noncopyable
{
public:
    tls_content() : current_sub_session(0)
    {
    }

    memory_sub_session* current_sub_session;
};

/// Initializes thread local storage. Should be called before usage of memory manager in threads. Must not be called again before tlsClean.
void tls_init();
/// Deallocates resources for thread local storage. Must not be called when threads are still using memory manager, before tlsInit or twice.
void tls_clean();

/// Initializes thread for usage with memory manager. Must be called in each thread before usage of memory manager in it.
void memory_thread_init_if_needed();

/// Returns TLS content for current thread.

inline tls_content* tls()
{
    return reinterpret_cast<tls_content*> (pthread_getspecific(tls_key));
}

}
}

#endif