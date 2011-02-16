/*
 * (C) Copyright 2011 Marek Dopiera
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

#include <config/config.h>
#include <debug/asserts.h>
#include <log/log.h>
#include <journal/journal_recovery_file.h>
#include <util/multi_buffer.h>
#include <ctime>

namespace coherent {
namespace journal {
namespace unittests {

using namespace std;
using namespace boost;
using namespace log4cxx;
using namespace coherent::config;
using namespace coherent::log;
using namespace util;

shared_ptr<multi_buffer> random_multibuffer()
{
//tworzymy losowy bufor
	multi_buffer::buffer_list buf_list;
	return shared_ptr<multi_buffer>( new multi_buffer(buf_list,0,0));
}

void journal_recovery_file_simple()
{
	journal_recovery_file file("log.file");
}

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.journal")->setLevel(log_TRACE);

	
	journal_recovery_file_simple();
	return 0;
}

} // namespace unittests
} // namespace journal
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::journal::unittests::start_test(argc, argv);
}
