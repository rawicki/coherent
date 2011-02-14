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
#include <journal/journal_file.h>
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

void journal_file_simple()
{
	srand(1234);
	journal_file file("log.file", 10);
	journal_block block;

	owner_id_t owner;
	handle_t handle = 0;
	bool append_ok;

	map<owner_id_t,set<handle_t> > open_reqs;
	map<owner_id_t,map<handle_t,shared_ptr<multi_buffer> > > all_reqs;

	while(true)
	{
		owner = rand() % 4 + 1;

		if( rand() % 2 == 0)
		{//insert
			handle++;
			open_reqs[owner].insert(handle);
		
			all_reqs[owner][handle] = random_multibuffer();

			append_ok = block.append_insert_record(owner,handle,*all_reqs[owner][handle]);
			if( block.ready_to_sync() || !append_ok)
			{
				file.write_block(block);

				//tutaj iterujemy po nastepnym bloku
				//i sprawdzamy czy jego rekordy(jesli jakiekolwiek) sa zgodne z tymi w all_reqs
			}

			if( !append_ok)
			{
				r_assert(
					block.append_insert_record(owner,handle,*all_reqs[owner][handle]),
					"could not append record"
					);
			}
		}
		else
		{//erase
			if( open_reqs[owner].empty())
			{
				continue;
			}

			handle = *(open_reqs[owner].begin());
			
			append_ok = block.append_erase_record(owner,handle);

			if( block.ready_to_sync() || !append_ok)
			{
				//podobnie jak przy insercie
			}

			if( !append_ok)
			{
			}
		

			open_reqs[owner].erase(handle);
		}

	}
}

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.journal")->setLevel(log_TRACE);	

	
	journal_file_simple();
	return 0;
}

} // namespace unittests
} // namespace journal
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::journal::unittests::start_test(argc, argv);
}
