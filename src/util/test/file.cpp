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

#include <util/file.h>
#include <config/config.h>
#include <debug/asserts.h>
#include <log/log.h>

namespace coherent {
namespace util {
namespace unittests {

using namespace std;
using namespace log4cxx;
using namespace coherent::config;
using namespace coherent::log;

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.buffercache")->setLevel(log_TRACE);	

	typedef auto_ptr<file> file_ptr;

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


	return 0;
}

} // namespace unittests
} // namespace util
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::util::unittests::start_test(argc, argv);
}
