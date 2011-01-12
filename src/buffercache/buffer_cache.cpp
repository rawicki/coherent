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

#include <boost/make_shared.hpp>

#include <buffercache/buffer_cache.h>

namespace coherent {
namespace buffercache {

using namespace std;
using namespace util;

//==== helpers =================================================================

template <class T>
inline bool on_list(list<T> l, typename list<T>::value_type val)
{
	return find(l.begin(), l.end(), val) != l.end();
}

//==== buffer cache ============================================================

buffer_cache::buffer_cache(
	uint32_t n_buffers,
	uint32_t buffer_size,
	uint32_t alignment
) :
	n_buffers(n_buffers),
	buffer_size(buffer_size),
	alignment(alignment)
{
	for (uint32_t i = 0; i < n_buffers; ++i)
		this->unbound.push_front(
			bound_buffer(boost::make_shared<buffer>(buffer_size, alignment))
		);
}

buffer_cache::buffer_it buffer_cache::get_frame(cached_file & file)
{
	r_assert(!unbound.empty() || !ready.empty(), "no available frames");

	bool const use_unbound = !this->unbound.empty();
	buffer_it const frame = use_unbound
		? this->unbound.begin()
		: this->ready.begin();

	this->in_use.splice(
		this->in_use.end(),
		use_unbound ? this->unbound : this->ready,
		frame
		);
	frame->ensure_uniq(this->alignment);
	if (!use_unbound) {
		frame->unbind();
	}
	frame->bind(file);
	frame->use(this->alignment);
	return frame;
}

void buffer_cache::make_ready(buffer_it const buf)
{
	d_assert(
		on_list(this->in_use, *buf),
		"can't make a frame ready if it's not in use"
		);
	//insert to the end, so that LRU works
	this->ready.splice(
		this->ready.end(),
		this->in_use,
		buf
		);
	buf->stop_using();
}

void buffer_cache::make_unbound(buffer_it const buf)
{
	d_assert(
		on_list(this->ready, *buf),
		"can't make a frame ready if it's not ready"
		);
	this->unbound.splice(
		this->unbound.end(), //could be begin as well
		this->ready,
		buf
		);
}

void buffer_cache::register_hit(buffer_it const buf)
{
	d_assert(
		on_list(this->ready, *buf),
		"can't hit a frame which is not ready"
		);
	this->ready.splice(
		this->ready.end(),
		this->ready,
		buf
		);
}

} // namespace buffercache
} // namespace coherent

