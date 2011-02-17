/*
 * (C) Copyright 2010 Xilexio
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

#include <sys/mman.h>
#include <memory_manager/manager.h>
#include <memory_manager/session.h>
#include <memory_manager/sub_session.h>
#include <debug/asserts.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace coherent
{
namespace memory_manager
{

typedef boost::unique_lock<boost::recursive_mutex> scoped_lock;

memory_sub_session::memory_sub_session(bool autostart):
parent(memory_session::current()), unfrozen_threads_count(-1), allocated_bytes(0)
{
    internal_init(autostart);
}

memory_sub_session::memory_sub_session(memory_session* session, bool autostart):
parent(session), unfrozen_threads_count(-1), allocated_bytes(0)
{
    internal_init(autostart);
}

memory_sub_session::~memory_sub_session()
{
    end();

    if (allocated_bytes != 0)
	LOG(WARN, "memory leak - not all memory was deallocated while destroying memory sub session\n");
}

memory_sub_session* memory_sub_session::current()
{
    return memory_session::current_sub_session.get();
}

void memory_sub_session::begin()
{
    activate();

    if (!frozen.get())
    {
	scoped_lock l(mutex);
	frozen.reset(new bool(unfrozen_threads_count == 0));

	if (!*frozen.get())
	{
	    if (unfrozen_threads_count == -1)
		unfrozen_threads_count = 1;
	    else
		++unfrozen_threads_count;
	}
    }
}

void memory_sub_session::end()
{
    deactivate();
}

void memory_sub_session::freeze()
{
    d_assert(frozen.get());

    if (*frozen.get())
	return;

    scoped_lock l(mutex);
    --unfrozen_threads_count;
    frozen.reset(new bool(true));

    if (unfrozen_threads_count == 0)
    {
	for (std::map<byte*, size_t>::const_iterator i = allocs.begin(); i != allocs.end(); ++i)
	{
	    if (posix_madvise(i->first, i->second, POSIX_MADV_DONTNEED))
		LOG(WARN, "madvise error on freezing\n");
	}

	scoped_lock pl(parent->mutex);
	parent->ram_allocated_bytes -= allocated_bytes;
    }
}

bool memory_sub_session::unfreeze()
{
    d_assert(frozen.get());

    if (!*frozen.get())
	return true;

    scoped_lock l(mutex);
    if (unfrozen_threads_count == 0)
    {
	scoped_lock pl(parent->mutex);
	if (parent->ram_allocated_bytes + allocated_bytes > parent->ram_limit_bytes)
	    return false;

	parent->ram_allocated_bytes += allocated_bytes;

	++unfrozen_threads_count;
	frozen.reset(new bool(false));

	for (std::map<byte*, size_t>::const_iterator i = allocs.begin(); i != allocs.end(); ++i)
	{
	    if (posix_madvise(i->first, i->second, POSIX_MADV_NORMAL))
		LOG(ERROR, "madvise error on unfreezing\n");
	}
    }

    return true;
}

bool memory_sub_session::is_frozen() const
{
    return *frozen.get();
}

void memory_sub_session::set_current()
{
    activate();
}

size_t memory_sub_session::get_allocated_bytes() const
{
    scoped_lock l(mutex);
    return allocated_bytes;
}

memory_session* memory_sub_session::get_parent() const
{
    return parent;
}

void memory_sub_session::internal_init(bool autostart)
{
    scoped_lock l(parent->mutex);
    parent->sub_sessions.insert(this);

    if (autostart)
	begin();
}

void memory_sub_session::activate()
{
    memory_session::current_sub_session.reset(this);
}

void memory_sub_session::deactivate()
{
    if (memory_session::current_sub_session.get() == this)
	memory_session::current_sub_session.reset();
}

byte* memory_sub_session::allocate(size_t needed_bytes)
{
    size_t page_size = memory_manager::instance->get_page_size();

    byte* res;

    if (needed_bytes > memory_manager::instance->get_max_small_alloc_bytes())
    {
	size_t bytes = (needed_bytes + page_size - 1) / page_size * page_size;

	byte* p = reinterpret_cast<byte*> (mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

	if (p == MAP_FAILED)
	    throw std::bad_alloc();

	add_alloc(p, bytes);

	res = p;
    }
    else
    {
	std::pair<size_t, byte*> optimal_chunk = smallest_sufficent_free_small_chunk(needed_bytes);

	size_t remainder_size;

	if (optimal_chunk.first == 0)
	{
	    size_t bytes = memory_manager::instance->get_single_small_alloc_bytes();

	    byte* p = reinterpret_cast<byte*> (mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

	    if (p == MAP_FAILED)
		throw std::bad_alloc();

	    add_alloc(p, bytes);
	    add_small_alloc(p, needed_bytes);

	    res = p;

	    remainder_size = bytes - needed_bytes;
	}
	else
	{
	    remainder_size = optimal_chunk.first - needed_bytes;

	    add_small_alloc(optimal_chunk.second, needed_bytes);
	    remove_free_small_chunk(optimal_chunk.second);

	    res = optimal_chunk.second;
	}

	if (remainder_size != 0)
	{
	    byte* remainder_pointer = res + needed_bytes;

	    add_free_small_chunk(remainder_pointer, remainder_size);
	}
    }

    return res;
}

void memory_sub_session::deallocate(byte* p, size_t bytes)
{
    if (is_small_alloc(p))
    {
	// removing chunk
	remove_small_alloc(p);

	std::pair<byte*, size_t> chunk_alloc = free_small_chunk_alloc(p);
	std::pair<byte*, size_t> remainder = add_merge_remove_free_small_chunk(p, bytes);

	if (chunk_alloc.second == remainder.second)
	{
	    if (munmap(chunk_alloc.first, chunk_alloc.second))
		LOG(WARN, "unmap error\n");

	    remove_alloc(chunk_alloc.first);
	}
	else
	{
	    add_free_small_chunk(remainder.first, remainder.second);
	}
    }
    else
    {
	if (munmap(reinterpret_cast<void*> (p), bytes))
	    LOG(WARN, "unmap error\n");

	remove_alloc(p);
    }
}

void memory_sub_session::add_alloc(byte* p, size_t bytes)
{
    allocated_bytes += bytes;
    allocs.insert(std::make_pair(p, bytes));

    parent->ram_allocated_bytes += bytes;
    parent->total_allocated_bytes += bytes;
    parent->allocs.insert(std::make_pair(p, this));
}

void memory_sub_session::remove_alloc(byte* p)
{
    std::map<byte*, size_t>::iterator i = allocs.find(p);
    d_assert(i != allocs.end());

    size_t bytes = i->second;

    allocated_bytes -= bytes;
    allocs.erase(i);

    parent->ram_allocated_bytes -= bytes;
    parent->total_allocated_bytes -= bytes;
    parent->allocs.insert(std::make_pair(p, this));
}

std::pair<size_t, byte*> memory_sub_session::smallest_sufficent_free_small_chunk(size_t bytes)
{
    std::set<std::pair<size_t, byte*> >::iterator i = free_small_chunks_inv.upper_bound(std::make_pair(bytes, (byte*) 0));

    if (i == free_small_chunks_inv.end())
	return std::make_pair(0, (byte*) 0);
    else
	return *i;
}

void memory_sub_session::add_small_alloc(byte* p, size_t bytes)
{
    small_allocs.insert(std::make_pair(p, bytes));
}

void memory_sub_session::remove_small_alloc(byte* p)
{
    std::set<std::pair<byte*, size_t> >::const_iterator i = small_allocs.upper_bound(std::make_pair(p, 0));
    d_assert(i != small_allocs.end());
    d_assert(i->first == p);

    small_allocs.erase(i);
}

bool memory_sub_session::is_small_alloc(byte* p) const
{
    std::set<std::pair<byte*, size_t> >::iterator sa = small_allocs.upper_bound(std::make_pair(p, 0));

    return sa != small_allocs.end() && sa->first == p;
}

void memory_sub_session::add_free_small_chunk(byte* p, size_t bytes)
{
    free_small_chunks.insert(std::make_pair(p, bytes));
    free_small_chunks_inv.insert(std::make_pair(bytes, p));
}

void memory_sub_session::remove_free_small_chunk(byte* p)
{
    free_small_chunks_inv.erase(std::make_pair(free_small_chunks[p], p));
    free_small_chunks.erase(p);
}

std::pair<byte*, size_t> memory_sub_session::free_small_chunk_alloc(byte* p) const
{
    std::map<byte*, size_t>::const_iterator chunk_alloc = allocs.upper_bound(p);
    d_assert(chunk_alloc != allocs.begin());
    --chunk_alloc;

    return *chunk_alloc;
}

std::pair<byte*, size_t> memory_sub_session::add_merge_remove_free_small_chunk(byte* p, size_t bytes)
{
    std::pair<byte*, size_t> chunk_alloc = free_small_chunk_alloc(p);

    size_t remainder_size = bytes;
    byte* remainder_pointer = p;

    if (!free_small_chunks.empty())
    {
	// adding and merging free space
	std::map<byte*, size_t>::iterator next = free_small_chunks.upper_bound(remainder_pointer);

	if (next != free_small_chunks.begin())
	{
	    std::map<byte*, size_t>::iterator previous = next;
	    --previous;

	    if (previous->first >= chunk_alloc.first)
	    {
		d_assert(remainder_pointer > previous->first);

		size_t diff = remainder_pointer - previous->first;
		d_assert(diff >= previous->second);

		if (diff == previous->second)
		{
		    remainder_pointer = previous->first;
		    remainder_size += previous->second;

		    free_small_chunks_inv.erase(std::make_pair(previous->second, previous->first));
		    free_small_chunks.erase(previous);

		    next = free_small_chunks.upper_bound(remainder_pointer);
		}
	    }
	}

	if (next != free_small_chunks.end() && remainder_size != chunk_alloc.second)
	{
	    d_assert(next->first > remainder_pointer);

	    size_t diff = next->first - remainder_pointer;

	    if (diff == remainder_size)
	    {
		remainder_size += next->second;

		free_small_chunks_inv.erase(std::make_pair(next->second, next->first));
		free_small_chunks.erase(next);
	    }
	}
    }

    return std::make_pair(remainder_pointer, remainder_size);
}

}
}
