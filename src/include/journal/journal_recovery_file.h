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

#ifndef JOURNAL_RECOVERY_FILE_H_9243
#define JOURNAL_RECOVERY_FILE_H_9243


#include <boost/noncopyable.hpp>
#include <util/multi_buffer.h>
#include <journal/abs_journal.h>

namespace coherent {
	namespace journal {
		typedef abs_journal::owner_id_t owner_id_t;
		typedef abs_journal::handle_t handle_t;
		typedef uint32_t rec_num_t;
		typedef uint64_t checksum_t;
		typedef std::map<owner_id_t,
			std::map<handle_t, boost::shared_ptr<util::multi_buffer> > > dispatch_t;


		class journal_recovery_file : private boost::noncopyable
		{
				int fd;
// 				typedef std::map<owner_id_t,
// 					std::pair<handle_t, boost::shared_ptr<util::multi_buffer> > > dispatch_t;

			public:
			
				journal_recovery_file(
					const std::string & pathname
				);
				~journal_recovery_file();
// 				void write_dispatch_map_to_file(dispatch_map &);
    
		};
	} // namespace journal
} // namespace coherent

#endif
