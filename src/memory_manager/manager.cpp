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

namespace coherent {
namespace memory_manager {

const char* OutOfTotalMemory::what() const throw() {
    return "Insufficent total memory left for memory reservation.";
}

MemoryManager* MemoryManager::instance = 0;

MemoryManager::~MemoryManager() {
    tlsClean();
    assert(!pthread_mutex_destroy(&reservedBytesMutex));
}

void MemoryManager::reserveBytes(size_t bytes) throw(OutOfTotalMemory) {
    ScopedMutex rm(&reservedBytesMutex);

    if (reservedBytes + bytes > limitBytes)
        throw OutOfTotalMemory();

    reservedBytes += bytes;
}

void MemoryManager::freeBytes(size_t bytes) throw() {
    ScopedMutex rm(&reservedBytesMutex);

    assert(bytes <= reservedBytes); // TODO na locku lub w ogóle

    reservedBytes -= bytes;
}

void MemoryManager::init(const config::global_config& conf) {
    assert(!instance);

    instance = new MemoryManager(conf);
}

MemoryManager::MemoryManager(const config::global_config& conf):
reservedBytes(0), limitBytes(conf.memory_manager.initialLimitBytes), defaultSessionLimitBytes(conf.memory_manager.defaultSessionLimitBytes) {
    pageSize = sysconf(_SC_PAGESIZE);

    tlsInit();
    assert(!pthread_mutex_init(&reservedBytesMutex, 0));
}

}
}