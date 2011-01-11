/*
 * (C) Copyright 2010 Cezary Bartoszuk, Michał Stachurski, Rafał Rawicki
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


#ifndef __COHERENT_NETSERVER_THREADPOOL_H__
#define __COHERENT_NETSERVER_THREADPOOL_H__


#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <deque>
#include "queue.h"


namespace coherent
{
namespace netserver
{

class thread_pool
{
public:
    typedef ::boost::function<void()> task_t;
private:
    int threads_num;
    queue<task_t, ::std::deque, no_wrapper> task_queue;
    ::boost::thread_group threads;
    void worker_thread_body();
public:
    thread_pool(const int pool_size);
    ~thread_pool();
    void run();
    void interrupt();
    void schedule(task_t task);
};

void defer(thread_pool::task_t task);

class join_point
{
private:
	int counter;
	thread_pool::task_t callback;
	::boost::mutex mutex;
public:
	typedef ::boost::shared_ptr<join_point> shared_ptr_t;
	join_point(const int number_to_join, thread_pool::task_t all_joined_callback);
	~join_point();
	void join();
	void lazy_join();
	static shared_ptr_t create(const int number_to_join, thread_pool::task_t all_joined_callback);
};

}  // namespace netserver
}  // namespace coherent


#endif
