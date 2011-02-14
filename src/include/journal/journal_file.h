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

#ifndef JOURNAL_FILE_H_9213
#define JOURNAL_FILE_H_9213
#include <boost/noncopyable.hpp>
#include <util/multi_buffer.h>
#include <journal/abs_journal.h>

namespace coherent {
	namespace journal {
		typedef abs_journal::owner_id_t owner_id_t;
		typedef abs_journal::handle_t handle_t;
		typedef uint32_t rec_num_t;
		typedef uint64_t checksum_t;

		const size_t BLOCK_SIZE = 2048;

		enum rec_type_t { INSERT_REC, ERASE_REC};

		struct journal_record : public util::virtual_dest
		{
			size_t rec_size;
			rec_type_t rec_type;
			owner_id_t owner;
			handle_t handle;
		};

		struct journal_block_header : public util::virtual_dest
		/*bloki maja ustalona dlugosc*/
		{
			size_t rec_sum_size;
			checksum_t checksum;
		};

		struct journal_block : public util::virtual_dest
		{
			journal_block_header header;
			char * recs_data;

			bool append_insert_record(
					owner_id_t,
					handle_t, 			
					util::multi_buffer const &
				);
			bool append_erase_record(
					owner_id_t,
					handle_t
				);

			bool ready_to_sync() const;
			void clear();
		};

#define block_record(block,block_offset) ((journal_record *)((block).recs_data + (block_offset)))
#define for_each_record(block,offset)\
		for(	size_t offset = 0;\
			offset < (block).header.rec_sum_size;\
			offset += block_record(block,offset)->rec_size)

		typedef boost::shared_ptr<journal_record> journal_record_ptr;

		class journal_file : private boost::noncopyable
		{
			int fd;
			public:
			
			journal_file(
					const std::string & pathname,
					uint32_t blocks_num
				);
			~journal_file();
			void write_block( journal_block &);
		};
	} // namespace journal
} // namespace coherent

#endif
