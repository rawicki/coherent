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

void read_test()
{
	LOG(INFO, "===== read_test");

	//ensure it doesn't exist
	file_ptr f = file::create("read_test", 0, 0600);
	f->close();

	//bypass any file realted code
	int fd = open("read_test", O_WRONLY);
	r_assert(fd != -1, "open failed: " << fd);

	auto_ptr<test_pattern> pat(new test_pattern());

	ssize_t res = write(fd, pat->pattern, file_size);
	r_assert(res >= 0, "pwrite: " << errno << " " << strerror(errno));
	r_assert(
		static_cast<ssize_t>(file_size) == res,
		"short write: " << res << " " << file_size
		);
	
	fd = close(fd);
	r_assert(fd == 0, "close: " << errno << " " << strerror(errno));

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

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.util")->setLevel(log_TRACE);	

	is_pow_2_test();
	align_test();
	open_test();
	read_test();

	return 0;
}

} // namespace unittests
} // namespace util
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::util::unittests::start_test(argc, argv);
}
