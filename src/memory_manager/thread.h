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

namespace coherent {
namespace memory_manager {

class MemorySession;

/// thread local storage key
static pthread_key_t tlsKey;

/// Container for thread-specific data used for memory management.
class TLSContent: private boost::noncopyable {
public:
    TLSContent(): currentSession(0) {}

    MemorySession* currentSession;
};

/// Initializes thread local storage. Should be called before usage of memory manager in threads. Must not be called again before tlsClean.
void tlsInit();
/// Deallocates resources for thread local storage. Must not be called when threads are still using memory manager, before tlsInit or twice.
void tlsClean();

/// Initializes thread for usage with memory manager. Must be called in each thread before usage of memory manager in it.
void memoryThreadInitIfNeeded();

/// Returns TLS content for current thread.
inline TLSContent* tls() {
    return static_cast<TLSContent*>(pthread_getspecific(tlsKey));
}

}
}

#endif