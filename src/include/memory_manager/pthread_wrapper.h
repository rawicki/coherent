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

namespace coherent
{
namespace memory_manager
{

class scoped_rwlock_read : private boost::noncopyable
{
public:

    scoped_rwlock_read(pthread_rwlock_t* initialized_lock) throw () : lock(initialized_lock)
    {
	assert(!pthread_rwlock_rdlock(lock));
    }

    ~scoped_rwlock_read() throw ()
    {
	assert(!pthread_rwlock_unlock(lock));
    }

private:
    pthread_rwlock_t* lock;
};

class scoped_rwlock_write : private boost::noncopyable
{
public:

    scoped_rwlock_write(pthread_rwlock_t* initialized_lock) throw () : lock(initialized_lock)
    {
	assert(!pthread_rwlock_wrlock(lock));
    }

    ~scoped_rwlock_write() throw ()
    {
	assert(!pthread_rwlock_unlock(lock));
    }

private:
    pthread_rwlock_t* lock;
};

class scoped_mutex : private boost::noncopyable
{
public:

    scoped_mutex(pthread_mutex_t* initialized_mutex) throw () : mutex(initialized_mutex)
    {
	assert(!pthread_mutex_lock(mutex));
    }

    ~scoped_mutex() throw ()
    {
	assert(!pthread_mutex_unlock(mutex));
    }

private:
    pthread_mutex_t* mutex;
};

}
}

#endif
