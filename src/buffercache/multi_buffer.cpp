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

#include <cstdlib>
#include <exception>
#include <cstring>

#include <debug/asserts.h>
#include <log/log.h>
#include <buffercache/multi_buffer.h>

namespace coherent {
namespace buffercache {

using namespace std;
using namespace coherent::debug;

#ifdef VALGRIND
//it sometimes has some issues with memcpy - probably memcpy aligns what it
//copies and valgrind doesn't like it

void memcpy(char *dst, char const * src, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		*(dst++) = *(src++);
}
#endif

//=========== buffer implementation ============================================

inline char * alloc_aligned(uint32_t size, uint32_t alignment)
{
	char * mem;
	if (alignment == buffer::NO_ALIGNMENT)
		mem = reinterpret_cast<char*>(malloc(size));
	else
		if (posix_memalign(reinterpret_cast<void**>(&mem), alignment, size))
			mem = 0;
	if (mem == 0)
		throw bad_alloc();
	return mem;
}

buffer::buffer(uint32_t size, uint32_t alignment) :
	data(alloc_aligned(size, alignment)),
	size(size)
{
}

buffer::~buffer()
{
	free(this->data);
}

//=========== multi_buffer implementation ======================================

multi_buffer::multi_buffer(
	buffer_list & buffers,
	uint32_t size,
	uint32_t left_off
) :
	size(size),
	left_off(left_off)
{
	d_assert((size == 0 && left_off == 0) || (left_off < size),
		"invalid use, size=" << size << ", left_off=" << left_off
	);

	buffers.swap(this->buffers);
#ifndef NDEBUG
	uint32_t real_size = 0;
	for (
		buffer_list::const_iterator i = this->buffers.begin();
		i != this->buffers.end();
		++i
	)
		real_size += (*i)->get_size();

	d_assert(left_off + size <= real_size,
		"invalid use, left_off=" << left_off << ", size=" << size
		<< ", real_size=" << real_size
	);
#endif

	d_assert(
		this->buffers.empty()
		|| this->buffers[0]->get_size() > left_off,
		"invalid use: buffer_size=" << this->buffers[0]->get_size() <<
		", left_off=" << left_off
	);
	LOG(TRACE, "Created multi_buffer size=" << size << " off=" << left_off);
}

multi_buffer::multi_buffer(multi_buffer const & o) :
	buffers(o.buffers),
	size(o.size),
	left_off(o.left_off)
{
	LOG(TRACE, "Copied multi_buffer size=" << size << " off=" << left_off);
}

template<bool READ>
void multi_buffer::do_rw(
	char * buf,
	uint32_t req_size,
	uint32_t req_off,
	uint32_t align
)
{
	d_assert(this->size >= req_size + req_off, "invalid use, req_size=" <<
		req_size << ", req_off=" << req_off << ", size=" << this->size
	);

	LOG(TRACE, "r/w to multi_buffer, size=" << req_size << ", off="
		<< req_off
	);

	if (!req_size)
		return; //perfectly legal situation
	d_assert(!this->buffers.empty(), "buffers list empty, req_size=" <<
		req_size << ", req_off=" << req_off
	);

	uint32_t off = 0;
	uint32_t size_left = req_size;
	
	for (buffer_list::iterator it = this->buffers.begin(); size_left; ++it) {
		d_assert(it != this->buffers.end(), "too long request?");
		uint32_t const off_in_buf =
			(it == this->buffers.begin()) ? this->left_off : 0;
		uint32_t const buf_size = (*it)->get_size() - off_in_buf;
		d_assert(buf_size, "empty buffer? off=" << off);
		if (off + buf_size > req_off) {

			uint32_t const buf_start = (req_off > off) ? (req_off - off) : 0;
			d_assert(
				buf_size > buf_start,
				"how? buf_size=" << buf_size << ", buf_start=" << buf_start
			);
			uint32_t const size = min(size_left, buf_size - buf_start);
			d_assert(off + buf_start >= req_off,
				"can't be, off=" << off << ", req_off=" << req_off
			);
			uint32_t const out_off = off + buf_start - req_off;
			d_assert(
				out_off + size <= req_size,
				"trying to write past the buffer, out_off=" << out_off <<
				", size=" << size << ", req_size=" << req_size
			);
			if (READ)
				memcpy(
					buf + out_off,
					(*it)->get_data() + off_in_buf + buf_start,
					size
				);
			else {
				if (!it->unique())
				{
					//ensure C-O-W
					buffer_ptr new_buf(new buffer((*it)->get_size(), align));
					memcpy(
						new_buf->get_data(),
						(*it)->get_data(),
						(*it)->get_size()
					);
					*it = new_buf;
				}
				memcpy(
					(*it)->get_data() + off_in_buf + buf_start,
					buf + out_off,
					size
				);
			}
			d_assert(size_left >= size, "mismatch, size_left=" << size_left <<
				", size=" << size
			);
			size_left -= size;
		}
		off += buf_size;
	}
}

template
void multi_buffer::do_rw<true>(
	char * buf,
	uint32_t size,
	uint32_t off,
	uint32_t align
);
template
void multi_buffer::do_rw<false>(
	char * buf,
	uint32_t size,
	uint32_t off,
	uint32_t align
);

} // namespace buffercache
} // namespace coherent

