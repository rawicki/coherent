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

#ifndef MULTI_BUFFER_H_2363
#define MULTI_BUFFER_H_2363

#include <stdint.h>

#include <vector>

#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

namespace coherent {
namespace util {

class buffer : private boost::noncopyable
{
public:
	enum
	{
		NO_ALIGNMENT = 0
	};

	buffer(uint32_t size, uint32_t alignment = NO_ALIGNMENT);
	~buffer();
	
	inline char * get_data();
	inline uint32_t get_size();

private:
	char * data;
	uint32_t size;

};

class multi_buffer
{
public:
	typedef boost::shared_ptr<buffer> buffer_ptr;
	typedef	std::vector<buffer_ptr> buffer_list;

	//invalidates buffers
	multi_buffer(buffer_list & buffers, uint32_t size, uint32_t left_off);
	multi_buffer(multi_buffer const & o);
	inline void read(
		char * buf,
		uint32_t size,
		uint32_t off
	);
	inline void write(
		char const * buf,
		uint32_t size,
		uint32_t off,
		//what alignment shall frames allocated by COW have
		uint32_t align = buffer::NO_ALIGNMENT
	);
	inline uint32_t get_size();

private:
	multi_buffer & operator=(multi_buffer const &); //not implemented on purpose

	template<bool READ>
	void do_rw(
		char * buf,
		uint32_t size,
		uint32_t off,
		uint32_t align = buffer::NO_ALIGNMENT
	);

	buffer_list buffers;
	uint32_t size;
	uint32_t left_off;
};

//================ IMPLEMENTATION ==============================================

uint32_t buffer::get_size()
{
	return this->size;
}

char * buffer::get_data()
{
	return this->data;
}

void multi_buffer::read(
	char * buf,
	uint32_t size,
	uint32_t off
)
{
	this->do_rw<true>(buf, size, off);
}

void multi_buffer::write(
	char const * buf,
	uint32_t size,
	uint32_t off,
	uint32_t align
)
{
	this->do_rw<false>(const_cast<char *>(buf), size, off, align);
}

uint32_t multi_buffer::get_size()
{
	return this->size;
}


} //namespace util
} //namespace coherent

#endif /* MULTI_BUFFER_H_2363 */
