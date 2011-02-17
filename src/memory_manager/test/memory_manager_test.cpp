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

#include <cmath>
#include <ctime>
#include <pthread.h>
#include <memory_manager/manager.h>
#include <memory_manager/session.h>
#include <memory_manager/allocator.h>
#include <config/config.h>
#include <debug/asserts.h>
#include <vector>
#include <math.h>

using namespace std;
using namespace coherent::memory_manager;
using namespace coherent::config;

static const int threads_count = 4;
static const int writer_bytes = 16 * 1024 * 1024;
static const int sorting_threads_count = 4;
static const int sorted_bytes = 4 * 1024 * 1024;

// total memory usage for writer <= 2 * writer_bytes
// total memory usage for mergesort < threads_count * sorted_bytes * (5 + ceil(lg sorting_threads_count))

void* writer(void*)
{
    memory_session ms;

    try
    {
	const int alloc_size = writer_bytes / sizeof (int);

	int* a = allocate<int>(alloc_size);
	for (int i = 0; i < alloc_size; ++i)
	    a[i] = i;
	deallocate(a, alloc_size);

	vector<int, coherent::memory_manager::allocator<int> > v;
	for (int i = 0; i < alloc_size / 8; ++i)
	    v.push_back(3);
	v.resize(alloc_size);
	v.clear();

	LOG(INFO, "writer ended allocating and writing data\n");
    }
    catch (out_of_session_ram_memory& ex)
    {
	r_assert(false, "out_of_session_ram_memory");
    }
    catch (out_of_session_total_memory& ex)
    {
	r_assert(false, "out_of_session_total_memory");
    }
    catch (out_of_total_memory)
    {
	r_assert(false, "out_of_total_memory");
    }

    r_assert(ms.get_total_allocated_bytes() == 0, "Not all memory was deallocated in writer");

    return 0;
}

void merge(int* begin, int* end, int* target)
{
    const int len = end - begin;
    int* left = begin;
    int* middle = begin + len / 2;
    int* right = middle;

    for (int i = 0; i < len; ++i)
    {
	if (right == end || (left != middle && *left <= *right))
	{
	    target[i] = *left;
	    ++left;
	}
	else
	{
	    target[i] = *right;
	    ++right;
	}
    }
}

void merge_sort(int* begin, int* end, memory_session* ms)
{
    if (end - begin <= 1)
	return;

    const int len = end - begin;
    int* buffer = allocate<int>(len);

    for (int i = 0; i < len; ++i)
    {
	buffer[i] = begin[i];
    }

    merge_sort(buffer, buffer + len / 2, ms);
    merge_sort(buffer + len / 2, buffer + len, ms);

    merge(buffer, buffer + len, begin);

    deallocate(buffer, len);
}

struct merge_sub_sorter_data
{
    memory_session* ms;
    int* begin;
    int* end;
    int threadsSpawn;
};

void* merge_sub_sorter(void* data)
{
    try
    {
	merge_sub_sorter_data* d = static_cast<merge_sub_sorter_data*> (data);

	d->ms->begin();

	if (d->threadsSpawn == 1)
	{
	    merge_sort(d->begin, d->end, d->ms);
	}
	else
	{
	    const int len = d->end - d->begin;
	    int* buffer = allocate<int>(len);
	    for (int i = 0; i < len; ++i)
		buffer[i] = d->begin[i];

	    pthread_t* threads = new pthread_t[2];
	    merge_sub_sorter_data* spawn_data = allocate<merge_sub_sorter_data>(2);
	    for (int i = 0; i < 2; ++i)
	    {
		spawn_data[i].ms = d->ms;
		spawn_data[i].begin = buffer + i * len / 2;
		spawn_data[i].end = buffer + (i + 1) * len / 2;
		spawn_data[i].threadsSpawn = d->threadsSpawn / 2 + d->threadsSpawn % 2 * i;
		r_assert(!pthread_create(threads + i, 0, merge_sub_sorter, spawn_data + i), "Could not create thread");
	    }

	    for (int i = 0; i < 2; ++i)
		pthread_join(threads[i], 0);

	    deallocate(spawn_data, 2);

	    merge(buffer, buffer + len, d->begin);

	    deallocate(buffer, len);
	}

	d->ms->end();
    }
    catch (out_of_session_ram_memory& ex)
    {
	r_assert(false, "out_of_session_ram_memory");
    }
    catch (out_of_session_total_memory& ex)
    {
	r_assert(false, "out_of_session_total_memory");
    }
    catch (out_of_total_memory)
    {
	r_assert(false, "out_of_total_memory");
    }

    return 0;
}

void* merge_sorter(void*)
{
    memory_session ms;

    try
    {
	ms.set_ram_limit_bytes(sorted_bytes * (5 + (size_t) ceil(log2((double) sorting_threads_count))));
	ms.set_total_limit_bytes(sorted_bytes * (5 + (size_t) ceil(log2((double) sorting_threads_count))));

	const int data_size = sorted_bytes / sizeof (int);

	int* data = allocate<int>(data_size);
	for (int i = 0; i < data_size; ++i)
	{
	    data[i] = rand() - RAND_MAX / 2;
	}

	LOG(INFO, "merge_sorter initialized and generated " << sorted_bytes << " B");

	merge_sub_sorter_data* d = allocate<merge_sub_sorter_data>(1);
	d->ms = &ms;
	d->begin = data;
	d->end = data + data_size;
	d->threadsSpawn = sorting_threads_count;
	pthread_t thread;
	r_assert(!pthread_create(&thread, 0, merge_sub_sorter, d), "Could not create thread");

	pthread_join(thread, 0);

	deallocate(d, 1);

	for (int i = 0; i < data_size - 1; ++i)
	{
	    r_assert(data[i] <= data[i + 1], "Data was not sorted correctly");
	}

	deallocate(data, data_size);

	LOG(INFO, "merge_sorter sorted " << sorted_bytes << " B");
    }
    catch (out_of_session_ram_memory& ex)
    {
	r_assert(false, "out_of_session_ram_memory");
    }
    catch (out_of_session_total_memory& ex)
    {
	r_assert(false, "out_of_session_total_memory");
    }
    catch (out_of_total_memory)
    {
	r_assert(false, "out_of_total_memory");
    }

    r_assert(ms.get_total_allocated_bytes() == 0, "Not all memory was deallocated in merge_sorter");

    return 0;
}

int main(const int argc, const char *const *const argv)
{
    srand(time(0));

    scoped_test_enabler test_setup(argc, argv);

    memory_manager::init(*test_setup.get_config());
    LOG(INFO, "page size: " << memory_manager::instance->get_page_size());

    try
    {
	pthread_t* threads = new pthread_t[threads_count];

	{
	    for (int i = 0; i < threads_count; ++i)
		r_assert(!pthread_create(threads + i, 0, writer, 0), "Could not create thread");

	    for (int i = 0; i < threads_count; ++i)
		pthread_join(threads[i], 0);
	}

	{
	    for (int i = 0; i < threads_count; ++i)
		r_assert(!pthread_create(threads + i, 0, merge_sorter, 0), "Could not create thread");

	    for (int i = 0; i < threads_count; ++i)
		pthread_join(threads[i], 0);
	}
    }
    catch (out_of_session_ram_memory& ex)
    {
	r_assert(false, "out_of_session_ram_memory");
    }
    catch (out_of_session_total_memory& ex)
    {
	r_assert(false, "out_of_session_total_memory");
    }
    catch (out_of_total_memory)
    {
	r_assert(false, "out_of_total_memory");
    }

    return 0;
}
