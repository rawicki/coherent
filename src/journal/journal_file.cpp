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

#include <journal/journal_file.h>

namespace coherent {
	namespace journal {

		using namespace std;
		using namespace util;
		using namespace boost;

		void journal_block::clear()
		{
		}

		bool journal_block::append_insert_record(
				owner_id_t owner,
				handle_t handle,
				util::multi_buffer const & buf)
		{
			return false;
		}
		bool journal_block::append_erase_record(
				owner_id_t owner,
				handle_t handle)
		{
			return false;
		}

		bool journal_block::ready_to_sync() const
		{
			return false;
		}

		journal_file::journal_file(
				const string & pathname,
				uint32_t blocks_num)
		{
		}

		journal_file::~journal_file()
		{
		}

		void journal_file::write_block( journal_block & b)
		{
		}
	} // namespace journal 
} // namespace coherent

