/*
 * (C) Copyright 2011 Marek Dopiera
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

#ifndef WORKER_POOL_H_3392
#define WORKER_POOL_H_3392

#include <queue>

#include <boost/optional.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <debug/asserts.h>

namespace coherent {
namespace util {

template <class T>
class sync_queue
{
public:
	sync_queue();
	~sync_queue(); //for assertions
	void push(T const & t);
	boost::optional<T> pop();
	void no_more_input();

private:
	boost::mutex mutex;
	boost::condition_variable cond;
	std::queue<T> q;
	bool no_more_data;
};

template <class T>
class worker_pool;

template <class T>
class worker
{
public:
	worker(worker_pool<T> & worker_pool);
	virtual ~worker();
	void operator()() const;
protected:
	virtual void handle(T const & t) const = 0; //I want workers to be stateless
private:
	worker_pool<T> & pool;
};

template <class T>
class worker_factory
{
public:
	typedef boost::thread* worker_ptr;

	virtual worker_ptr create_worker(worker_pool<T> & pool) const = 0;
	virtual ~worker_factory();
};

template <class T>
class worker_pool
{
public:
	worker_pool();
	~worker_pool();
	void start(uint32_t num_workers, worker_factory<T> const & factory);
	void stop();
	void schedule_work(T const & work);
private:
	friend class worker<T>;

	typedef boost::ptr_vector<boost::thread> workers_t;
	workers_t workers;
	sync_queue<T> queue;
};

//========== IMPLEMENTATION ====================================================

//========== sync_queue ========================================================

template <class T>
sync_queue<T>::sync_queue() : no_more_data(false)
{
}

template <class T>
sync_queue<T>::~sync_queue()
{
	d_assert(
		no_more_data && q.empty(),
		"trying to destruct a queue but empty=" << q.empty() << " no_more_data="
		<< no_more_data
		);
}

template <class T>
void sync_queue<T>::push(T const & t)
{
	boost::mutex::scoped_lock lock(this->mutex);
	d_assert(!no_more_data, "trying to push to a closing queue");

	this->q.push(t);
	this->cond.notify_one();
}

template <class T>
boost::optional<T> sync_queue<T>::pop()
{
	boost::mutex::scoped_lock lock(this->mutex);
	while (q.empty() && !no_more_data)
	{
		this->cond.wait(lock);
	}
	if (!q.empty()) {
		T res = this->q.front();
		this->q.pop();
		return boost::make_optional(res);
	} else {
		d_assert(no_more_data, "what?!");
		return boost::optional<T>();
	}
		
}

template <class T>
void sync_queue<T>::no_more_input()
{
	boost::mutex::scoped_lock lock(this->mutex);
	this->no_more_data = true;
	this->cond.notify_all();
}

//========== worker ============================================================

template <class T>
worker<T>::worker(worker_pool<T> & pool) : pool(pool)
{
}

template <class T>
worker<T>::~worker()
{
}

template <class T>
void worker<T>::operator()() const
{
	LOG(TRACE, "thread " << pthread_self() << " has been started");
	boost::optional<T> res = this->pool.queue.pop();
	while (res)
	{
		LOG(TRACE, "thread " << pthread_self() << " handle work");
		this->handle(res.get());
		res = this->pool.queue.pop();
	}
	LOG(TRACE, "thread " << pthread_self() << " finishing");
}

//========== worker_factory ====================================================

template <class T>
worker_factory<T>::~worker_factory()
{
}

//========== worker_pool =======================================================

template <class T>
worker_pool<T>::worker_pool()
{
}

template <class T>
worker_pool<T>::~worker_pool()
{
	d_assert(this->workers.empty(), "there are still workers in dtor");
}

template <class T>
void worker_pool<T>::start(uint32_t num_workers, worker_factory<T> const & factory)
{
	LOG(DEBUG, "worker pool starting " << num_workers << " threads");
	//FIXME exception safety
	for (uint32_t i = 0; i < num_workers; ++i)
		this->workers.push_back(factory.create_worker(*this));
	LOG(DEBUG, "worker pool started");
}

template <class T>
void worker_pool<T>::stop()
{
	LOG(DEBUG, "worker pool stopping");
	this->queue.no_more_input();
	LOG(TRACE, "informed threads");
	for (
		workers_t::iterator i = this->workers.begin();
		i != this->workers.end();
		++i
		)
	{
		i->join();
	}
	LOG(DEBUG, "worker pool has joined all threads");
	this->workers.clear();
}

template <class T>
void worker_pool<T>::schedule_work(T const & work)
{
	this->queue.push(work);
}


} // namespace util
} // namespace coherent

#endif /* WORKER_POOL_H_3392 */
