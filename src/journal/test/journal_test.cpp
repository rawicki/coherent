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
#include <journal/journal.h>
#include <journal/test/journal_mocks.h>
#include <util/multi_buffer.h>

namespace coherent {
namespace journal {
namespace unittests {

using namespace std;
using namespace boost;
using namespace log4cxx;
using namespace coherent::config;
using namespace coherent::log;


void in_mem_journal_simple()
{
	LOG(INFO, "===== in_mem_journal_simple");
	in_mem_journal j;
	sync_journal_wrapper sj(j);

	int owners_num = 5;
	int size = 3;
	util::multi_buffer::buffer_list buflist;
	for( int i = 0; i < owners_num; i++){
		buflist.push_back(util::multi_buffer::buffer_ptr(new util::buffer(size)));
	}

	util::multi_buffer buf(buflist,owners_num*size,0);
	vector<journal::handle_t> handlers;
	journal::handle_t handle;
	for( int i = 0; i < size*owners_num; i++){
		cerr << i << endl;
		LOG(INFO, "insert " << i%owners_num);
		handle = sj.insert(i%owners_num, buf);
		sj.erase(i%owners_num,handle);
	}

}

void journal_simple()
{
	LOG(INFO, "starting simple journal");
	journal j;
	sync_journal_wrapper sj(j);

	int owners_num = 5;
	int size = 3;
	util::multi_buffer::buffer_list buflist;
	for( int i = 0; i < owners_num; i++){
		buflist.push_back(util::multi_buffer::buffer_ptr(new util::buffer(size)));
	}

	util::multi_buffer buf(buflist,owners_num*size,0);
	vector<journal::handle_t> handlers;
	journal::handle_t handle;
	for( int i = 0; i < size*owners_num; i++){
		cerr << i << endl;
		LOG(INFO, "insert " << i%owners_num);
                handle = sj.insert(i%owners_num, buf);
                LOG(INFO, "erase " << i%owners_num);
                sj.erase(i%owners_num,handle);
        }
	markthis();	
}

int start_test(const int argc, const char *const *const argv)
{
	scoped_test_enabler test_setup(argc, argv);

	Logger::getLogger("coherent.journal")->setLevel(log_TRACE);	

	//in_mem_journal_simple();
	journal_simple();
	return 0;
}

} // namespace unittests
} // namespace journal
} // namespace coherent

int main(const int argc, const char * const * const argv)
{
	return coherent::journal::unittests::start_test(argc, argv);
}
