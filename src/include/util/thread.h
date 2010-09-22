/*
 * (C) Copyright 2010 Marek Dopiera
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

#ifndef THREAD_H_3295
#define THREAD_H_3295

#include <boost/noncopyable.hpp>

#include <util/misc.h>
#include <config/cmake_config.h>
#include <debug/asserts.h>

namespace coherent {
namespace util {
	

#ifdef HAVE_SYNC_BOOL_COMPARE_AND_SWAP_INT
	#define TAS_IMPL_GNU_BUILTIN

#elif  defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

	#ifdef __i386__
	#define TAS_IMPL_ASM_i386
	#else
	#define TAS_IMPL_ASM_amd64
	#endif
#else
#error Your compiler is not gcc-4.1 or newer and your architecture is not\
	i386 neither amd64,	so I dont know how to provide the test and set\
	operation.
#endif

namespace detail {

template <class T>
class scoped_lock_base : private boost::noncopyable {

private:
	T &mutex;
	bool locked;

protected:
	explicit scoped_lock_base(T & mutex); //doesn't try to lock the lock

public:
	~scoped_lock_base();
	void lock();
	void unlock();
	bool try_lock();
	bool owns_lock();

};

template <class T>
class scoped_lock : public scoped_lock_base<T>
{
public:
	explicit scoped_lock(T & mutex);
};

template <class T>
class scoped_try_lock : public scoped_lock_base<T>
{
public:
	explicit scoped_try_lock(T & mutex);
};

#ifdef TAS_IMPL_GNU_BUILTIN

#define SPIN_IMPL int
#define SPIN_IMPL_LOCKED 187
#define SPIN_IMPL_UNLOCKED 245
#define SPIN_IMPL_INITIALIZER SPIN_IMPL_UNLOCKED

inline void SPIN_IMPL_LOCK(volatile SPIN_IMPL *lock) 
{
	while (unlikely(!__sync_bool_compare_and_swap(lock, SPIN_IMPL_UNLOCKED, SPIN_IMPL_LOCKED))) {}
}

inline void SPIN_IMPL_UNLOCK(volatile SPIN_IMPL *lock)
{
	bool res = __sync_bool_compare_and_swap(lock, SPIN_IMPL_LOCKED, SPIN_IMPL_UNLOCKED);
	d_assert(res, "Spin lock implementation problem");
}

inline void SPIN_IMPL_INIT(volatile SPIN_IMPL *lock)
{
	*lock = SPIN_IMPL_UNLOCKED;
}

inline bool SPIN_IMPL_TRYLOCK(volatile SPIN_IMPL *lock)
{
	return __sync_bool_compare_and_swap(lock, SPIN_IMPL_UNLOCKED, SPIN_IMPL_LOCKED);
}

#elif defined(TAS_IMPL_ASM_i386) || defined(TAS_IMPL_ASM_amd64)

#define SPIN_IMPL int
#define SPIN_IMPL_LOCKED 187
#define SPIN_IMPL_UNLOCKED 245
#define SPIN_IMPL_INITIALIZER SPIN_IMPL_UNLOCKED

#define EXCHANGE(lock, locker) {\
	asm volatile ("xchgl %1, (%0);" \
			:"=r"(lock), "=r"(locker) \
			:"0"(lock), "1"(locker) \
			);\
}

inline void SPIN_IMPL_LOCK(volatile SPIN_IMPL *lock) 
{
	volatile SPIN_IMPL locker = SPIN_IMPL_LOCKED;
	do {
		EXCHANGE(lock, locker);
	} while (unlikely(locker == SPIN_IMPL_LOCKED));
	d_assert(locker == SPIN_IMPL_UNLOCKED, "lock corruption: locker=" << locker);
}

inline void SPIN_IMPL_UNLOCK(volatile SPIN_IMPL *lock)
{
	volatile SPIN_IMPL unlocker = SPIN_IMPL_UNLOCKED;
	EXCHANGE(lock, unlocker);
}

inline void SPIN_IMPL_INIT(volatile SPIN_IMPL *lock)
{
	*lock = SPIN_IMPL_UNLOCKED;
}

inline bool SPIN_IMPL_TRYLOCK(volatile SPIN_IMPL *lock)
{
	volatile SPIN_IMPL locker = SPIN_IMPL_LOCKED;
	EXCHANGE(lock, locker);
	return locker == SPIN_IMPL_UNLOCKED;
}

#endif /* spinlock type */

template <class T>
inline scoped_lock_base<T>::scoped_lock_base(T & mutex) : mutex(mutex), locked(false)
{
}


template <class T>
inline scoped_lock_base<T>::~scoped_lock_base()
{
	if (this->locked)
		mutex.unlock();
}

template <class T>
inline void scoped_lock_base<T>::lock()
{
	d_assert(!this->locked, "Trying to acquire a locked lock");
	this->mutex.lock();
	this->locked = true;
}

template <class T>
inline void scoped_lock_base<T>::unlock()
{
	d_assert(this->locked, "Trying to unlock an already unlocked lock.");
	this->mutex.unlock();
	this->locked = false;
}

template <class T>
inline bool scoped_lock_base<T>::try_lock()
{
	d_assert(!this->locked, "Trying to acquire an already locked lock");
	this->locked = this->mutex.try_lock();
	return this->locked;
}

template <class T>
inline bool scoped_lock_base<T>::owns_lock()
{
	return this->locked;
}


template <class T>
inline scoped_lock<T>::scoped_lock(T & mutex) : scoped_lock_base<T>(mutex)
{
	this->lock();
}

template <class T>
scoped_try_lock<T>::scoped_try_lock(T & mutex) : scoped_lock_base<T>(mutex)
{
	this->try_lock();
}

} // namespace detail

class spin_mutex : private boost::noncopyable {
public:
	typedef detail::scoped_lock<spin_mutex> scoped_lock;
	typedef detail::scoped_try_lock<spin_mutex> scoped_try_lock;

	friend class detail::scoped_lock_base<spin_mutex>;
	friend class detail::scoped_try_lock<spin_mutex>;
	friend class detail::scoped_lock<spin_mutex>;

	inline spin_mutex() 
	{
		detail::SPIN_IMPL_INIT(&lock_impl);
	}


private:
	volatile SPIN_IMPL lock_impl;
	
	inline void lock()
	{
		detail::SPIN_IMPL_LOCK(&(this->lock_impl));
	}

	inline void unlock()
	{
		detail::SPIN_IMPL_UNLOCK(&(this->lock_impl));
	}

	inline bool try_lock()
	{
		return detail::SPIN_IMPL_TRYLOCK(&(this->lock_impl));
	}

};


} // namespace util
} // namespace coherent

#endif /* THREAD_H_3295 */
