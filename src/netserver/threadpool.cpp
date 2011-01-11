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


#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include "threadpool.h"


namespace coherent
{
namespace netserver
{

boost::thread_specific_ptr<thread_pool> my_thread_pool;

thread_pool::thread_pool(const int pool_size)
  : threads_num(pool_size)
{
}

thread_pool::~thread_pool()
{
}

void thread_pool::run()
{
	::boost::function<void()> worker = ::boost::bind(&thread_pool::worker_thread_body, this);
	for(int i=0; i<threads_num; i++)
	{
		threads.create_thread(worker);
	}
}

void thread_pool::interrupt()
{
	threads.interrupt_all();
}

void thread_pool::schedule(task_t task)
{
	task_queue.push(task);
}

void thread_pool::worker_thread_body()
{
	my_thread_pool.reset(this);
	for(;;)
	{
		::boost::this_thread::interruption_point();
		task_t task = task_queue.pop();
		task();
	}
}

void defer(thread_pool::task_t task)
{
	my_thread_pool->schedule(task);
}

join_point::join_point(const int number_to_join, thread_pool::task_t all_joined_callback)
  : counter(number_to_join),
    callback(all_joined_callback)
{
}

join_point::~join_point()
{
}

void join_point::join()
{
	::boost::lock_guard< ::boost::mutex> lock(mutex);
	counter -= 1;
	if(counter == 0) {
		callback();
	}
}

void join_point::lazy_join()
{
	::boost::lock_guard< ::boost::mutex> lock(mutex);
	counter -= 1;
	if(counter == 0) {
		defer(callback);
	}
}

join_point::shared_ptr_t join_point::create(const int number_to_join, thread_pool::task_t all_joined_callback)
{
	shared_ptr_t ptr(new join_point(number_to_join, all_joined_callback));
	return ptr;
}

}  // namespace netserver
}  // namespace coherent
