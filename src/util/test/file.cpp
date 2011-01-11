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
#include <sys/fcntl.h>

#include <cstring>
#include <cstdlib>

#include <boost/shared_array.hpp>

#include <util/file.h>
#include <config/config.h>
#include <debug/asserts.h>
#include <log/log.h>

namespace coherent {
namespace util {
namespace unittests {

using namespace std;
using namespace boost;
using namespace log4cxx;
using namespace coherent::config;
using namespace coherent::log;

typedef auto_ptr<file> file_ptr;

static uint32_t const file_size = 4 * 1024 * 1024 + 1; //make sure that it's unaligned

struct test_pattern
{
	test_pattern()
	{
		for (uint32_t i = 0; i < file_size; ++i)
		{
			pattern[i] = random();
		}
	}

	char pattern[file_size];
};

void is_pow_2_test()
{
	LOG(INFO, "===== is_pow_2_test");
	r_assert(is_pow2(1), "error");
	r_assert(is_pow2(2), "error");
	r_assert(is_pow2(4), "error");
	r_assert(is_pow2(8), "error");
	r_assert(is_pow2(131072), "error");

	r_assert(!is_pow2(0), "error");
	r_assert(!is_pow2(-2), "error");
	r_assert(!is_pow2(3), "error");
	r_assert(!is_pow2(7), "error");
	r_assert(!is_pow2(131071), "error");

	r_assert(is_pow2(1U << (sizeof(unsigned) - 1)), "error");
	r_assert(!is_pow2(numeric_limits<int>::max()), "error");
}

void align_test()
{
	LOG(INFO, "===== align_test");

	r_assert(align_down(0, 16) == 0, "error");
	r_assert(align_down(1, 16) == 0, "error");
	r_assert(align_down(8, 16) == 0, "error");
	r_assert(align_down(15, 16) == 0, "error");

	r_assert(align_up(0, 16) == 0, "error");
	r_assert(align_up(1, 16) == 16, "error");
	r_assert(align_up(8, 16) == 16, "error");
	r_assert(align_up(15, 16) == 16, "error");

	r_assert(align_down(53, 16) == 48, "error");
	r_assert(align_up(53, 16) == 64, "error");
}

void open_test()
{
	LOG(INFO, "===== open_test");

	file_ptr f1 = file::create("t1", 0, 0600);
	{
		bool exception_caught = false;
		try {
			file_ptr f2 = file::create("t1", 0, 0600);
		} catch (io_exception & ex) {
			LOG(INFO, "Expected exception: " << ex.what());
			exception_caught = true;
		}
		r_assert(exception_caught, "succeeded to create the same file twice!");
	}
	f1->close();
	file_ptr f2 = file::open("t1", 0);
	f2->close();
}

void check_range(
	file & f,
	test_pattern const & pat,
	uint32_t start,
	uint32_t end,
	bool expect_except = false
	)
{
	LOG(INFO, "checking range " << start << " " << end);
	if (!expect_except) {
		file::multi_buffer_ptr res = f.read(end - start, start);
		shared_array<char> buf = shared_array<char>(new char[end - start]);
		res->read(buf.get(), end - start, 0);

		r_assert(
			!memcmp(buf.get(), pat.pattern + start, end - start),
			"mismatch in range " << start << " " << end
			);
	} else {
		try {
			f.read(end - start, start);
		} catch (io_exception & ex) {
			LOG(INFO, "expected exception: " << ex.what());
			return;
		}
		r_assert(false, "expected exception not thrown");
	}
}

void prepare_file(string const & name, test_pattern const & pat)
{
	//bypass any file realted code
	int fd = open(name.c_str(), O_WRONLY);
	r_assert(fd != -1, "open failed: " << fd);


	ssize_t res = write(fd, pat.pattern, file_size);
	r_assert(res >= 0, "pwrite: " << errno << " " << strerror(errno));
	r_assert(
		static_cast<ssize_t>(file_size) == res,
		"short write: " << res << " " << file_size
		);
	
	fd = close(fd);
	r_assert(fd == 0, "close: " << errno << " " << strerror(errno));

}

void read_test()
{
	LOG(INFO, "===== read_test");

	//ensure it doesn't exist
	file_ptr f = file::create("read_test", 0, 0600);
	f->close();

	auto_ptr<test_pattern> pat(new test_pattern());

	prepare_file("read_test", *pat);

	f = file::open("read_test", O_RDONLY);

	check_range(*f, *pat, 0, file_size);
	check_range(*f, *pat, 0, 0);

	for (uint32_t i = 0; i < 1000; i++) {
		uint32_t start = random() % file_size;
		uint32_t end = start + random() % (file_size - start);

		check_range(*f, *pat, start, end);
	}

	//we expect an axception due to short read
	check_range(*f, *pat, file_size - 2, file_size + 1, true);
	check_range(*f, *pat, 2, file_size + 1, true);


	f->close();
}
typedef shared_ptr<multi_buffer> multi_buffer_ptr;

pair<uint32_t, multi_buffer_ptr> gen_random_buffer(test_pattern const & pat)
{
	uint32_t start = random() % (file_size - 1);
	uint32_t buf_start = start;

	multi_buffer::buffer_list bufs;

	uint32_t total_size = 0;
	for (uint32_t i = 0; i < 5; ++i) {
		uint32_t const shift = random() % 100 + 1;
		uint32_t const buf_end = min(file_size, buf_start + shift);

		bufs.push_back(multi_buffer::buffer_ptr(new buffer(shift)));
		memcpy(bufs.back()->get_data(), pat.pattern + buf_start, shift);

		total_size += shift;

		buf_start = buf_end;

		if (buf_end == file_size)
			break;

	}

	//add some shift from the left
	uint32_t const first_buf_shift = random() % bufs[0]->get_size();
	memset(bufs[0]->get_data(), 0, first_buf_shift);

	//cut some from the right
	uint32_t const last_buf_shift = (bufs.size() > 1)
		? (random() % bufs.back()->get_size())
		: 0;
	memset(
		bufs.back()->get_data() + bufs.back()->get_size() - last_buf_shift,
		0,
		last_buf_shift);

	uint32_t const res_off = start + first_buf_shift;
	multi_buffer_ptr res_buf(new multi_buffer(
			bufs,
			total_size - first_buf_shift - last_buf_shift, first_buf_shift
			)
		);
	return make_pair(res_off, res_buf);
}

void write_test()
{
	LOG(INFO, "===== write_test");

	file_ptr f = file::create("write_test", O_RDWR, 0600);
	try {
		//ensure it doesn't exist
		auto_ptr<test_pattern> pat(new test_pattern());

		prepare_file("write_test", *pat);

		for (int i = 0; i < 100; ++i) {
			pair<uint32_t, multi_buffer_ptr> const & to_write =
				gen_random_buffer(*pat);
			f->write(*to_write.second, to_write.first);
		}

		check_range(*f, *pat, 0, file_size);
	} catch (io_exception const & ex) {
		r_assert(false, "Unexpected exception caught: " << ex.what());
	}

	f->close();
}

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.util")->setLevel(log_TRACE);	

	is_pow_2_test();
	align_test();
	open_test();
	read_test();
	write_test();
	return 0;
}

} // namespace unittests
} // namespace util
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::util::unittests::start_test(argc, argv);
}
