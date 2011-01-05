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
	for(;;)
	{
		::boost::this_thread::interruption_point();
		task_t task = task_queue.pop();
		task();
	}
}


}  // namespace netserver
}  // namespace coherent
