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
#include <cstdio>
#include <vector>
#include <math.h>

using namespace std;
using namespace coherent::memory_manager;
using namespace coherent::config;

static const int threadsCount = 4;
static const int writerBytes = 16 * 1024 * 1024;
static const int sortingThreadsCount = 4;
static const int sortedBytes = 256 * 1024;

// total memory usage for writer <= 2 * writerBytes
// total memory usage for mergesort < threadsCount * sortedBytes * (5 + ceil(lg sortingThreadsCount))

void* writer(void*) {
    MemorySession ms;

    try {
        const int allocSize = writerBytes / sizeof(int);

        int* a = allocate<int>(allocSize);
        for (int i = 0; i < allocSize; ++i)
            a[i] = i;
        deallocate(a, allocSize);

        vector<int, Allocator<int> > v;
        for (int i = 0; i < allocSize / 8; ++i)
            v.push_back(3);
        v.resize(allocSize);
        v.clear();

        printf("writer ended allocating and writing data\n");
    } catch (OutOfSessionMemory& ex) {
        r_assert(false, "OutOfSessionMemory");
    } catch (OutOfTotalMemory) {
        r_assert(false, "OutOfTotalMemory");
    }

    r_assert(ms.getAllocatedBytes() == 0, "Not all memory was deallocated");

    return 0;
}

void merge(int* begin, int* end, int* target) {
    const int len = end - begin;
    int* left = begin;
    int* middle = begin + len / 2;
    int* right = middle;

    for (int i = 0; i < len; ++i) {
        if (right == end || (left != middle && *left <= *right)) {
            target[i] = *left;
            ++left;
        } else {
            target[i] = *right;
            ++right;
        }
    }
}

void mergeSort(int* begin, int* end, MemorySession* ms) {
    if (end - begin <= 1)
        return;

    const int len = end - begin;
    int* buffer = allocate<int>(len);

    for (int i = 0; i < len; ++i) {
        buffer[i] = begin[i];
    }

    mergeSort(buffer, buffer + len / 2, ms);
    mergeSort(buffer + len / 2, buffer + len, ms);

    merge(buffer, buffer + len, begin);

    deallocate(buffer, len);
}

struct MergeSubSorterData {
    MemorySession* ms;
    int* begin;
    int* end;
    int threadsSpawn;
};

void* mergeSubSorter(void* data) {
    try {
        MergeSubSorterData* d = static_cast<MergeSubSorterData*>(data);

        d->ms->begin();

        if (d->threadsSpawn == 1) {
            mergeSort(d->begin, d->end, d->ms);
        } else {
            const int len = d->end - d->begin;
            int* buffer = allocate<int>(len);
            for (int i = 0; i < len; ++i)
                buffer[i] = d->begin[i];

            pthread_t* threads = new pthread_t[2];
            MergeSubSorterData* spawnD = allocate<MergeSubSorterData>(2);
            for (int i = 0; i < 2; ++i) {
                spawnD[i].ms = d->ms;
                spawnD[i].begin = buffer + i * len / 2;
                spawnD[i].end = buffer + (i + 1) * len / 2;
                spawnD[i].threadsSpawn = d->threadsSpawn / 2 + d->threadsSpawn % 2 * i;
                r_assert(!pthread_create(threads + i, 0, mergeSubSorter, spawnD + i), "Could not create thread");
            }

            for (int i = 0; i < 2; ++i)
                pthread_join(threads[i], 0);

            deallocate(spawnD, 2);

            merge(buffer, buffer + len, d->begin);

            deallocate(buffer, len);
        }

        d->ms->end();
    } catch (OutOfSessionMemory& ex) {
        r_assert(false, "OutOfSessionMemory");
    } catch (OutOfTotalMemory) {
        r_assert(false, "OutOfTotalMemory");
    }

    return 0;
}

void* mergeSorter(void*) {
    MemorySession ms;

    try {
        ms.setLimitBytes(sortedBytes * (5 + ceil(log2(sortingThreadsCount))));

        const int dataSize = sortedBytes / sizeof(int);

        int* data = allocate<int>(dataSize);
        for (int i = 0; i < dataSize; ++i) {
            data[i] = rand() - RAND_MAX / 2;
        }

        clockid_t cid;
        timespec startTs;
        pthread_getcpuclockid(pthread_self(), &cid);
        clock_gettime(cid, &startTs);
        printf("mergeSorter initialized and generated %dB in %4ld.%03ld\n", sortedBytes, startTs.tv_sec, startTs.tv_nsec / 1000000);

        pthread_t thread;
        MergeSubSorterData* d = allocate<MergeSubSorterData>(1);
        d->ms = &ms;
        d->begin = data;
        d->end = data + dataSize;
        d->threadsSpawn = sortingThreadsCount;
        r_assert(!pthread_create(&thread, 0, mergeSubSorter, d), "Could not create thread");

        pthread_join(thread, 0);

        deallocate(d, 1);

        for (int i = 0; i < dataSize - 1; ++i) {
            r_assert(data[i] <= data[i + 1], "Data was not sorted correctly");
        }

        deallocate(data, dataSize);

        timespec ts;
        pthread_getcpuclockid(pthread_self(), &cid);
        clock_gettime(cid, &ts);
        ts.tv_nsec -= startTs.tv_nsec;
        ts.tv_sec -= startTs.tv_sec;
        if (ts.tv_nsec < 0) {
            ts.tv_sec -= 1;
            ts.tv_nsec += 1000000000;
        }
        printf("mergeSorter sorted %dB in %4ld.%03ld\n", sortedBytes, ts.tv_sec, ts.tv_nsec / 1000000);
    } catch (OutOfSessionMemory& ex) {
        r_assert(false, "OutOfSessionMemory");
    } catch (OutOfTotalMemory) {
        r_assert(false, "OutOfTotalMemory");
    }

    r_assert(ms.getAllocatedBytes() == 0, "Not all memory was deallocated");

    return 0;
}

int main(const int argc, const char *const *const argv) {
    srand(time(0));

    scoped_test_enabler test_setup(argc, argv);

    MemoryManager::init(*test_setup.get_config());

    try {
        pthread_t* threads = new pthread_t[threadsCount];

        {
            for (int i = 0; i < threadsCount; ++i)
                r_assert(!pthread_create(threads + i, 0, writer, 0), "Could not create thread");

            for (int i = 0; i < threadsCount; ++i)
                pthread_join(threads[i], 0);
        }

        {
            for (int i = 0; i < threadsCount; ++i)
                r_assert(!pthread_create(threads + i, 0, mergeSorter, 0), "Could not create thread");

            for (int i = 0; i < threadsCount; ++i)
                pthread_join(threads[i], 0);
        }
    } catch (OutOfSessionMemory& ex) {
        r_assert(false, "OutOfSessionMemory");
    } catch (OutOfTotalMemory) {
        r_assert(false, "OutOfTotalMemory");
    }

    return 0;
}
