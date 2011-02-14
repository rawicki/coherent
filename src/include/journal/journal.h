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

#ifndef JOURNAL_H_5312
#define JOURNAL_H_5312

#include <stdint.h>

#include <exception>

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <util/misc.h>
#include <util/multi_buffer.h>
#include <util/worker_pool.h>
#include <journal/abs_journal.h>
#include <journal/journal_file.h>
#include <map>

namespace coherent {
	namespace journal {


		class journal : public abs_journal
		{
			public:

				struct internal_req :	public util::virtual_dest
			{
				virtual void execute(journal & ) = 0;
			};

				struct insert_req :	public internal_req,
				public boost::enable_shared_from_this<insert_req>
			{
				insert_req(
						owner_id_t owner,
						util::multi_buffer const & buf,
						insert_cb & cb
					  ) : owner(owner), buf(buf), cb(cb)
				{
				}
				virtual void execute(journal & );

				owner_id_t const owner;
				util::multi_buffer const & buf;
				insert_cb & cb;
			};

				struct erase_req :	public internal_req,
				public boost::enable_shared_from_this<erase_req>
			{
				erase_req(
						owner_id_t const owner,
						handle_t const handle,
						erase_cb & cb
					 ) : owner(owner), handle(handle), cb(cb)
				{
				}
				virtual void execute(journal & );

				owner_id_t const owner;
				handle_t const handle;
				erase_cb & cb;
			};

				struct cb_req : public internal_req
			{
				cb_req(
						owner_id_t owner,
						handle_t handle,
						bool success = true
				      ) : owner(owner), handle(handle), success(success)
				{
				}
				owner_id_t const owner;
				handle_t const handle;
				bool success;
			};

				struct insert_cb_req : public cb_req
			{
				insert_cb_req(
						owner_id_t owner,
						handle_t handle,
						bool success = true
					     ) : cb_req(owner,handle,success)
				{
				}
				virtual void execute(journal &);
			};

				struct erase_cb_req : public cb_req
			{
				erase_cb_req(
						owner_id_t owner,
						handle_t handle,
						bool success = true
					    ) : cb_req(owner,handle,success)
				{
				}
				virtual void execute(journal &);
			};

				typedef boost::shared_ptr<internal_req> req_ptr;//pomyslec nad requestem ktory nie bylby z najwyzszej klasy
				typedef boost::shared_ptr<cb_req> cb_req_ptr;

			private:
				friend class journal_thread;
				friend class journal_callback_worker;

				typedef std::map<std::pair<owner_id_t,handle_t>,
					boost::shared_ptr<insert_req> > insert_reqs_map_t;

				typedef std::map<std::pair<owner_id_t,handle_t>,
					boost::shared_ptr<erase_req> > erase_reqs_map_t;

				insert_reqs_map_t insert_reqs;
				erase_reqs_map_t erase_reqs;

				util::sync_queue<req_ptr> reqs;

				util::worker_pool<cb_req_ptr> cb_workers;
				boost::thread * j_thread;


				handle_t max_handle;
				
				std::string JOURNAL_FILE_PATH;
				size_t MAX_REC_SIZE;//pozniej wywalic stad
				uint32_t BLOCKS_NUM;
				boost::shared_ptr<journal_file> file;
				journal_block block_buf;

				void initialize_journal_file(
						const std::string & file_path,
						uint32_t blocks_num
					);
				void write_record( journal_record_ptr);

			public:

				journal();
				~journal();

				//XXX replace with something reasonable
				typedef std::exception recovery_except;

				virtual inline void insert(
						owner_id_t owner,
						util::multi_buffer const & buf,
						insert_cb & cb
						) throw()
				{
					reqs.push(req_ptr(new insert_req(owner,buf,cb)));
				}

				virtual inline void erase(
						owner_id_t owner,
						handle_t handle,
						erase_cb & cb
						) throw()
				{
					reqs.push(req_ptr(new erase_req(owner,handle,cb)));
				}

				// should be called before any insert or erase
				virtual void recover(
						recovery_dispatcher & dispatcher
						) throw(recovery_dispatcher);

		};

		class journal_thread : public util::virtual_dest
		{
			journal & j;

			public:
			journal_thread(journal & j) : j(j)
			{
			}
			void operator()() const;
		};

		class journal_callback_worker
			: public util::worker<journal::cb_req_ptr>
		{
			journal & j;

			public:
			journal_callback_worker( journal &j)
				: util::worker<journal::cb_req_ptr>(j.cb_workers), j(j)
			{
			}

			inline virtual void handle(journal::cb_req_ptr const &t) const
			{
				t->execute(this->j);
			}
		};


		class journal_callback_worker_factory
			: public util::worker_factory<journal::cb_req_ptr>
		{
			journal & j;

			public:
			journal_callback_worker_factory(journal &j)
				: j(j)
			{
			}

			virtual worker_ptr create_worker(
					util::worker_pool<journal::cb_req_ptr> & pool
					) const
			{
				return new boost::thread(journal_callback_worker(this->j));
			}
		};

	} // namespace journal 
} // namespace coherent

#endif /* JOURNAL_H_5316 */
