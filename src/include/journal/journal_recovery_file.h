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

#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/noncopyable.hpp>
#include <util/multi_buffer.h>
#include <journal/abs_journal.h>

namespace coherent {
	namespace journal {
		typedef abs_journal::owner_id_t owner_id_t;
		typedef abs_journal::handle_t handle_t;
		typedef uint32_t rec_num_t;
		typedef uint64_t checksum_t;

		class dispatch_map
		{
			private:
				friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
				template<class Archive>
				void serialize(Archive & ar, const unsigned int version)
				{
					ar & degrees;
					ar & minutes;
					ar & seconds;
				}
				int degrees;
				int minutes;
				float seconds;
			public:
				dispatch_map(){};
				dispatch_map(int d, int m, float s) :
					    degrees(d), minutes(m), seconds(s)
				{}
		};

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
				void write_dispatch_map_to_file(dispatch_map &);
    
		};
	} // namespace journal
} // namespace coherent

#endif
