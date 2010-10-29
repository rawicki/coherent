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

#ifndef BUFFER_CACHE_H_0304
#define BUFFER_CACHE_H_0304

#include <list>
#include <map>

#include <boost/make_shared.hpp>

#include <buffercache/multi_buffer.h>
#include <debug/asserts.h>

namespace coherent {
namespace buffercache {

class cached_file;
class buffer_cache;

//BC shall not become owner of cached files but only differentiate between them
typedef cached_file * cached_file_ptr;

class bound_buffer
{
public:
	typedef multi_buffer::buffer_ptr buffer_ptr;

	bound_buffer(buffer_ptr buffer);

	inline buffer_ptr const & get_buffer() const;
	inline cached_file & get_file() const;
	inline bool is_bound() const;
	inline void bind(cached_file & file);
	inline void unbind();
	inline void ensure_uniq(uint32_t alignment = buffer::NO_ALIGNMENT);

private:
	
	//HACK BEGIN
	//this is a nasty way to create a list of noncopyable elements;
	//unfortunatelly noncopyable has to be implemented manually, because
	//otherwise making friend wouldn't work
	friend class std::list<bound_buffer>;
	friend class __gnu_cxx::new_allocator<bound_buffer>;
	friend void buffer_list_filler(
		void * list,
		uint32_t n_buffers,
		uint32_t buffer_size,
		uint32_t alignment
	);
	bound_buffer(bound_buffer const &); //unimplemented
	bound_buffer & operator=(bound_buffer const & bound_buffer); //unimplemented
	//HACK END

	inline void use(uint32_t alignment = buffer::NO_ALIGNMENT);
	inline void stop_using();

	buffer_ptr buffer;
	cached_file_ptr file;
	bool is_being_used;
};

class buffer_cache
{
public:
	typedef std::list<bound_buffer> buffer_list;
	typedef buffer_list::iterator buffer_ptr;
	typedef buffer_list::const_iterator buffer_cptr;

	buffer_cache(
		uint32_t n_buffers,
		uint32_t buffer_size,
		uint32_t alignment = buffer::NO_ALIGNMENT
	);

private:
	buffer_list unbound;
	buffer_list in_use;
	buffer_list ready;

	uint32_t n_buffers;
	uint32_t buffer_size;
	uint32_t alignment;
};

class cached_file
{
public:
	cached_file(buffer_cache & bc);
private:

	typedef std::map<uint64_t, buffer_cache::buffer_ptr> file_cache_map;
	buffer_cache & bc;
};
//======= inline implementation ==================================================

//======= bound_buffer ===========================================================

bound_buffer::bound_buffer(buffer_ptr buffer) : 
	buffer(buffer),
	file(NULL)
{
}

bound_buffer::buffer_ptr const & bound_buffer::get_buffer() const
{
	return this->buffer;
}

cached_file & bound_buffer::get_file() const
{
	d_assert(this->is_bound(), "Dereferencing non-existent file");
	return *this->file;
}

bool bound_buffer::is_bound() const
{
	return this->file != NULL;
}

void bound_buffer::bind(cached_file & file)
{
	d_assert(!this->is_bound(), "binding an already bound buffer");
	this->file = &file;
}

void bound_buffer::unbind()
{
	d_assert(this->is_bound(), "can't unbind an unbound buffer");
	d_assert(!this->is_being_used, "can't unbind a used buffer");
	this->file = NULL;
}

void bound_buffer::ensure_uniq(uint32_t alignment)
{
	if (!this->buffer.unique())
	{
		buffer_ptr old_buf = this->buffer;
		this->buffer = boost::make_shared<buffercache::buffer>(
			old_buf->get_size(),
			alignment
		);
		memcpy(
			this->buffer->get_data(),
			old_buf->get_data(),
			old_buf->get_size()
		);
	}
}

void bound_buffer::use(uint32_t alignment)
{
	d_assert(this->is_bound(), "can't use an unbound buffer");
	d_assert(!this->is_being_used, "trying to use a used buffer");
	this->is_being_used = true;
}

void bound_buffer::stop_using()
{
	d_assert(this->is_being_used, "can't stop using an unused buffer");
	this->is_being_used = false;
}

//======= buffer pool ============================================================

} //namespace buffercache
} //namespace coherent

#endif /* BUFFER_CACHE_H_0304 */
