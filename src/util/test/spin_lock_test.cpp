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

#include <errno.h>

#include <util/misc.h>
#include <util/thread.h>
#include <debug/debug.h>
#include <config/config.h>
#include <log/log.h>

using namespace std;
using namespace coherent::util;
using namespace coherent::config;
using namespace coherent::log;

volatile int k = 0;
spin_mutex mutex;

unsigned const num_iteratrions = 100000000 / VALGRIND_SLOWDOWN / VALGRIND_SLOWDOWN;

void *incrementer(void *)
{
	LOG(DEBUG, "Thread started");
	for (unsigned i = 0; i < num_iteratrions; ++i) {
		spin_mutex::scoped_lock l(mutex);
		volatile int j = k;
		j++;
		k = j;
	}
	return NULL;
}

int main(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	int err;
	pthread_t thread1 = 0, thread2 = 0;
	{
		spin_mutex::scoped_lock l(mutex);
		err = pthread_create(&thread1, NULL, incrementer, NULL);
		r_assert(err == 0, "failed to create thread, errno=" << errno);
		err = pthread_create(&thread2, NULL, incrementer, NULL);
		r_assert(err == 0, "failed to create thread, errno=" << errno);
	}
	err = pthread_join(thread1, NULL);
	r_assert(err == 0, "failed to join thread, errno=" << errno);
	err = pthread_join(thread2, NULL);
	r_assert(err == 0, "failed to join thread, errno=" << errno);

	r_assert(k == num_iteratrions * 2, "Expected k=" << num_iteratrions * 2 << ", but k=" << k);
	return 0;
}
