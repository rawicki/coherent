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

#include <pthread.h>

#include <util/worker_pool.h>
#include <config/config.h>
#include <debug/asserts.h>
#include <log/log.h>

namespace coherent {
namespace util {
namespace unittests {

using namespace std;
using namespace boost;
using namespace log4cxx;
using namespace coherent::config;
using namespace coherent::log;

class dummy_worker : public worker<int>
{
public:
	dummy_worker(worker_pool<int> & pool) : worker<int>(pool)
	{
	}

	void handle(int const & i) const
	{
		LOG(DEBUG, "thread " << ::pthread_self() << " handling " << i);
		usleep(random() % 100000);
	}
};

class dummy_worker_factory : public worker_factory<int>
{
public:
	virtual worker_ptr create_worker(worker_pool<int> & pool) const
	{
		return new thread(dummy_worker(pool));
	}
};

void worker_pool_start_stop()
{
	LOG(INFO, "========== worker_pool_start_stop");
	worker_pool<int> pool;
	for (uint32_t i = 0; i < 3; ++i)
	{
		{
			dummy_worker_factory const fact;
			pool.start(10, fact);
		}
		pool.stop();
	}
}

void worker_pool_handle_some()
{
	LOG(INFO, "========== worker_pool_handle_some");
	worker_pool<int> pool;
	dummy_worker_factory const fact;
	pool.start(10, fact);

	for (uint32_t i = 0; i < 100; ++i)
	{
		pool.schedule_work(i);
	}

	usleep(random() % 100000);
	pool.stop();
}

void sync_queue_single_threaded_test()
{
	LOG(INFO, "========== sync_queue_single_threaded_test");
	sync_queue<int> q;
	q.push(5);
	q.push(10);
	q.push(15);
	r_assert(q.pop().get() == 5, "queue doesn't work");
	r_assert(q.pop().get() == 10, "queue doesn't work");
	q.no_more_input();
	r_assert(q.pop().get() == 15, "queue doesn't work");
	r_assert(!q.pop(), "what?");
}

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.util")->setLevel(log_TRACE);	

	sync_queue_single_threaded_test();
	worker_pool_start_stop();
	worker_pool_handle_some();

	return 0;
}

} // namespace unittests
} // namespace util
} // namespace coherent


int main(const int argc, const char * const * const argv)
{
	return coherent::util::unittests::start_test(argc, argv);
}

