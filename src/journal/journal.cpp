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

#include <journal/journal.h>

namespace coherent {
	namespace journal {

		using namespace std;
		using namespace util;
		using namespace boost;

		//====== recovery_except =======================================================




		//====== journal ===============================================================

		journal::journal()
		{
			JOURNAL_FILE_PATH = "journal.log";
			BLOCKS_NUM = 100;
			MAX_REC_SIZE = 1024;
			initialize_journal_file(JOURNAL_FILE_PATH,BLOCKS_NUM);

			journal_callback_worker_factory factory(*this);
			this->cb_workers.start(2,factory);

			j_thread = new boost::thread(journal_thread(*this));
		}

		journal::~journal()
		{
			LOG(TRACE, "thread " << pthread_self() << " is ~journal");

			reqs.no_more_input();
			this->cb_workers.stop();

			j_thread->join();
			delete j_thread;
		}

		void journal::recover(
				recovery_dispatcher & dispatcher
				) throw(recovery_dispatcher)
		{
		}

		void journal::initialize_journal_file(
				const std::string &file_path,
				uint32_t blocks_num)
		{
			/*
				Trzymamy dwa pliki.
				Pierwszy to plik z logami a, na ktorym dziala journal.
				Drugi to plik b, w ktorym sa wpisane stare niezamkniete rekordy (to sa rekordy po ktore nikt sie nie zglosil dispatcherem).
			*/
			file = shared_ptr<journal_file>(
				new journal_file(file_path,blocks_num));
		}

		//====== journal_thread =================================================

		void journal_thread::operator()() const
		{
			LOG(TRACE, "thread " << pthread_self() << " has been started");

			boost::optional<journal::req_ptr> res = j.reqs.pop();
			while (res)
			{
				res.get()->execute(j);
				res = j.reqs.pop();
			}

			LOG(TRACE, "thread " << pthread_self() << " finishing");
		}

		//====== requests ==============================================================

		void journal::insert_req::execute(journal & j)
		{
			if( this->buf.get_size() > j.MAX_REC_SIZE)
			{
				
			}

			handle_t handle = j.max_handle++;

			j.insert_reqs[make_pair(this->owner,handle)] = shared_from_this();

			//tutaj zapisujemy do pliku

			bool appended =
				j.block_buf.append_insert_record( this->owner, handle, this->buf);

			if( j.block_buf.ready_to_sync() || !appended )
			{
				j.file->write_block( j.block_buf);

				for_each_record(j.block_buf,offset)
				{
					journal_record * rec = block_record(j.block_buf,offset);
					LOG(TRACE, "owner:" << rec->owner << " handle:" << rec->handle);
				}

				j.block_buf.clear();
			}
			if( !appended )
			{
				r_assert(
					j.block_buf.append_insert_record( handle, this->owner, this->buf),
					"could not append record"
					);
			}
		}

		void journal::erase_req::execute(journal & j)
		{
			LOG(TRACE, "thread " << pthread_self() << " made a sync_req");

			j.erase_reqs[make_pair(this->owner,this->handle)] = shared_from_this();

			j.cb_workers.schedule_work( cb_req_ptr( new erase_cb_req(this->owner, this->handle)));
		}

		//====== cb_reqs ===============================================================

		void journal::insert_cb_req::execute( journal & j)
		{
			shared_ptr<insert_req> req =
				get_map_val(j.insert_reqs,make_pair(this->owner,this->handle));

			r_assert( req->owner == this->owner, "owner " << req->owner << " != " << this->owner);

			LOG(TRACE, "thread " << pthread_self() << " cb.insert_success");
			req->cb.insert_success( req->owner, req->buf, this->handle);
		}

		void journal::erase_cb_req::execute( journal & j)
		{
			shared_ptr<erase_req> req =
				get_map_val(j.erase_reqs, make_pair(this->owner,this->handle));
			r_assert( req->owner == this->owner, "owner " << req->owner << " != " << this->owner);

			LOG(TRACE, "thread " << pthread_self() << " cb.erase_success");
			req->cb.erase_success( req->owner, req->handle);
		}



	} // namespace journal 
} // namespace coherent

