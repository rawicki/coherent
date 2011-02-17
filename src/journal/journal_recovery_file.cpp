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

#include <log/log.h>
#include <util/multi_buffer.h>
#include <journal/journal_recovery_file.h>


#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>



namespace coherent {
namespace journal {

	using namespace std;
	using namespace util;
	using namespace boost;


	shared_ptr<multi_buffer> random_multibuffer(uint32_t size)
	{
		multi_buffer::buffer_list buf_list;
		return shared_ptr<multi_buffer>( new multi_buffer(buf_list,0,0));
	}

	journal_recovery_file::journal_recovery_file(
			const string & pathname
		)
	{    
	}

	journal_recovery_file::~journal_recovery_file()
	{
	}

// 	void journal_recovery_file::write_dispatch_map_to_file(dispatch_map &disp_map)
// 	{
// 	}

} // namespace journal 
} // namespace coherent
