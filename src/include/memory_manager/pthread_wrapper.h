/* 
 * File:   pthreads_wrapper.h
 * Author: xilexio
 *
 * Created on November 22, 2010, 10:41 PM
 */

#ifndef PTHREADS_WRAPPER_H
#define	PTHREADS_WRAPPER_H

#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <cassert>

namespace coherent {
namespace memory_manager {

class ScopedRWLockRead: private boost::noncopyable {
public:
    ScopedRWLockRead(pthread_rwlock_t* initializedLock) throw(): lock(initializedLock) {
        assert(!pthread_rwlock_rdlock(lock));
    }

    ~ScopedRWLockRead() throw() {
        assert(!pthread_rwlock_unlock(lock));
    }

private:
    pthread_rwlock_t* lock;
};

class ScopedRWLockWrite: private boost::noncopyable {
public:
    ScopedRWLockWrite(pthread_rwlock_t* initializedLock) throw(): lock(initializedLock) {
        assert(!pthread_rwlock_wrlock(lock));
    }

    ~ScopedRWLockWrite() throw() {
        assert(!pthread_rwlock_unlock(lock));
    }

private:
    pthread_rwlock_t* lock;
};

class ScopedMutex: private boost::noncopyable {
public:
    ScopedMutex(pthread_mutex_t* initializedMutex) throw(): mutex(initializedMutex) {
        assert(!pthread_mutex_lock(mutex));
    }

    ~ScopedMutex() throw() {
        assert(!pthread_mutex_unlock(mutex));
    }

private:
    pthread_mutex_t* mutex;
};

}
}

#endif
