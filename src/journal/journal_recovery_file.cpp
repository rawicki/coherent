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


// #include <boost/archive/text_oarchive.hpp>
// #include <boost/archive/text_iarchive.hpp>
#include <journal/journal_recovery_file.h>

namespace coherent {
	namespace journal {

		using namespace std;
		using namespace util;
		using namespace boost;

		journal_recovery_file::journal_recovery_file(
				const string & pathname
			)
		{
		  {	dispatch_map disp_map;
			// create and open an archive for input
			ifstream ifs(pathname.c_str());
			archive::text_iarchive ia(ifs);
			// read class state from archive
// 			ia >> disp_map;
			// archive and stream closed when destructors are called
		}
		}

		journal_recovery_file::~journal_recovery_file()
		{
		}

		void journal_recovery_file::write_dispatch_map_to_file(dispatch_map &disp_map)
		{
		}

	} // namespace journal 
} // namespace coherent

