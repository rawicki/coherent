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
#include <memory_manager/pthread_wrapper.h>
#include "thread.h"

namespace coherent {
namespace memory_manager {

MemorySession::MemorySession(): allocatedBytes(0) {
    limitBytes = MemoryManager::instance->getDefaultSessionLimitBytes();

    internalInit();
}

MemorySession::MemorySession(size_t startingLimitBytes): limitBytes(startingLimitBytes), allocatedBytes(0) {
    internalInit();
}

MemorySession::~MemorySession() {
    end();

    assert(!pthread_rwlock_destroy(&limitLock));
    assert(!pthread_rwlock_destroy(&allocLock));

    MemoryManager::instance->freeBytes(limitBytes);
}

MemorySession* MemorySession::current() {
    return tls()->currentSession;
}

void MemorySession::begin() {
    memoryThreadInitIfNeeded();
    activate();

    ScopedMutex am(&activeThreadsMutex);
    ++activeThreadsCount;
}

void MemorySession::end() {
    deactivate();

    ScopedMutex am(&activeThreadsMutex);
    assert(activeThreadsCount > 0);
    --activeThreadsCount;
}

void MemorySession::stop() {
    deactivate();

    ScopedMutex am(&activeThreadsMutex);
    assert(activeThreadsCount > 0);
    --activeThreadsCount;

    if (activeThreadsCount == 0) {
        ScopedRWLockRead al(&allocLock);
        
        for (std::map<void*, size_t>::const_iterator i = allocations.begin(); i != allocations.end(); ++i) {
            if (madvise(i->first, i->second, MADV_DONTNEED))
                printf("MADVISE ERROR\n"); // TODO zamienić na log
        }
    }
}

void MemorySession::resume() {
    {
        ScopedMutex am(&activeThreadsMutex);
        ++activeThreadsCount;

        if (activeThreadsCount == 1) {
            ScopedRWLockRead al(&allocLock);
            
            for (std::map<void*, size_t>::const_iterator i = allocations.begin(); i != allocations.end(); ++i) {
                if (madvise(i->first, i->second, MADV_NORMAL))
                    printf("MADVISE ERROR\n"); // TODO zamienić na log
            }
        }
    }

    activate();
}

size_t MemorySession::getLimitBytes() const {
    ScopedRWLockRead ll(&limitLock);
    return limitBytes;
}

void MemorySession::setLimitBytes(size_t bytes) {
    ScopedRWLockWrite ll(&limitLock);
    limitBytes = bytes;
}

size_t MemorySession::getAllocatedBytes() const {
    ScopedRWLockRead al(&allocLock);
    return allocatedBytes;
}

void MemorySession::internalInit() {
    MemoryManager::instance->reserveBytes(limitBytes);

    assert(!pthread_mutex_init(&activeThreadsMutex, 0));
    assert(!pthread_rwlock_init(&limitLock, 0));
    assert(!pthread_rwlock_init(&allocLock, 0));

    begin();
}

void MemorySession::activate() {
    TLSContent* tlsContent = tls();

    if (tlsContent->currentSession && tlsContent->currentSession != this)
        tlsContent->currentSession->stop();

    tlsContent->currentSession = this;
}

void MemorySession::deactivate() {
    TLSContent* tlsContent = tls();

    if (tlsContent->currentSession == this)
        tlsContent->currentSession = 0;
}

}
}