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

#include <buffercache/buffer_cache.h>

namespace coherent {
namespace buffercache {

using namespace std;

//HACK BEGIN - see header for details
void buffer_list_filler(
	void * list,
	uint32_t n_buffers,
	uint32_t buffer_size,
	uint32_t alignment
)
{
	buffer_cache::buffer_list * blist =
		reinterpret_cast<buffer_cache::buffer_list *>(list);

	for (uint32_t i = 0; i < n_buffers; ++i)
		blist->push_front(
			bound_buffer(boost::make_shared<buffer>(buffer_size, alignment))
		);
}
//HACK END

buffer_cache::buffer_cache(
	uint32_t n_buffers,
	uint32_t buffer_size,
	uint32_t alignment
) :
	n_buffers(n_buffers),
	buffer_size(buffer_size),
	alignment(alignment)
{
	//HACK BEGIN - see header for details
	buffer_list_filler(
		reinterpret_cast<void *>(&this->unbound),
		n_buffers,
		buffer_size,
		alignment
	);
	//HACK END
}


} // namespace buffercache
} // namespace coherent

