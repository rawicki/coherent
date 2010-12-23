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

#include <cstring>
#include <cstdlib>

#include <util/multi_buffer.h>
#include <config/config.h>
#include <debug/asserts.h>
#include <log/log.h>

namespace coherent {
namespace util {
namespace unittests {

using namespace log4cxx;
using namespace coherent::config;
using namespace coherent::log;

static uint32_t const buf_size = 32000;

struct test_pattern
{
	test_pattern() : buf(new char[buf_size])
	{
		for (uint32_t i = 0; i < buf_size; ++i)
			buf[i] = random();
	}

	test_pattern(test_pattern const & o) : buf(new char[buf_size])
	{
		memcpy(this->buf, o.buf, buf_size);
	}

	~test_pattern()
	{
		delete[] buf;
	}

	char * buf;
};

test_pattern pattern;

static void check_buffers(char const * b1, char const * b2, uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i)
		r_assert(
			b1[i] == b2[i],
			"mismatch at byte " << i << " expected " << static_cast<int>(b1[i])
			<< " but got " << static_cast<int>(b2[i])
		);
}

void single_test(uint32_t left_off)
{
	uint32_t cur_off = left_off;
	multi_buffer::buffer_list bufs;
	uint32_t total_size = 0;

	test_pattern pattern_save(pattern);

	while (true) {
		uint32_t const size = random() % 300 + 1;
		if (cur_off + size > buf_size)
			break;
		if (cur_off == left_off) {
			//first frame
			multi_buffer::buffer_ptr buf(new buffer(size + left_off));
			memcpy(buf->get_data(), pattern.buf, size + left_off);
			bufs.push_back(buf);
			LOG(TRACE, "Added buffer off=0" << " size=" << size + left_off);
		}
		else
		{
			multi_buffer::buffer_ptr buf(new buffer(size));
			memcpy(buf->get_data(), pattern.buf + cur_off, size);
			bufs.push_back(buf);
			LOG(TRACE, "Added buffer off=" << cur_off << " size=" << size);
		}
		total_size += size;
		cur_off += size;
	}
	LOG(TRACE, "left_off=" << left_off << ", total_size=" << total_size <<
		" cur_off=" << cur_off
	);
	multi_buffer mbuf(bufs, total_size, left_off);
	multi_buffer mbuf2(mbuf);

	//read whole mbuf
	{
		char tmp[total_size];
		mbuf.read(tmp, total_size, 0);
		check_buffers(pattern.buf + left_off, tmp, total_size);
	}

	for (uint32_t i = 0; i < 200; ++i)
	{
		uint32_t const start = random() % total_size + 1;
		uint32_t const size = (i == 100) ? 0 : (random() % (total_size - start));
			
		char buf[size];
		if (i % 2) {
			for (uint32_t i = 0; i < size; ++i)
				pattern.buf[i+left_off + start] = buf[i] = random();
			mbuf.write(buf, size, start);
		}
		else
		{
			mbuf.read(buf, size, start);
			check_buffers(pattern.buf + left_off + start, buf, size);
		}

	}
	{
		//verify c-o-w
		char tmp[total_size];
		mbuf2.read(tmp, total_size, 0);
		check_buffers(pattern_save.buf + left_off, tmp, total_size);
	}
}
int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.util")->setLevel(log_TRACE);	

	single_test(0);
	for (uint32_t i = 0; i < 20; ++i)
		single_test(random() % 100 + 1);

	return 0;
}

} // namespace unittests
} // namespace util
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::util::unittests::start_test(argc, argv);
}
