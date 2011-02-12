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
#include <journal/absjournal.h>
#include <iostream>
#include <map>

	template<class K,class V>
bool inline key_exists( const std::map<K,V> &m, const K &k)
{
	return m.find(k) != m.end();
}
	
template<class K,class V>
const V & get_map_val( const std::map<K,V> &m, const K &k)
{
	typename std::map<K,V>::const_iterator it = m.find(k);
	r_assert(
		it != m.end(),
		"Key does not exist"//byc moze jakis toString by sie przydal
		);
	return it->second;
}

#define traverse(coll,it) \
	for(	const_cast<__typeof( (coll).begin())> it = (cool).begin(); \
			it != (coll).end(); \
			it++)

namespace coherent {
	namespace journal {

		typedef uint32_t owner_id_t;
		typedef uint32_t handle_t;


		class journal : public abs_journal
		{

			boost::thread * in_mem_thread;
			boost::thread * sync_thread;

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
					  );
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
					 );
				virtual void execute(journal & );

				owner_id_t const owner;
				handle_t const handle;
				erase_cb & cb;
			};


			struct sync_req : public internal_req//sprawdzic o co chodzi z virtual destem
			{
				sync_req(
						owner_id_t owner,
						handle_t handle
				   );

				owner_id_t const owner;
				handle_t const handle;
			};

			struct insert_sync_req : public sync_req
			{
				insert_sync_req(
						owner_id_t owner,
						handle_t handle,
						const util::multi_buffer & buf
					  );
				virtual void execute(journal & );

				const util::multi_buffer & buf;
			};

			struct erase_sync_req : public sync_req
			{
				erase_sync_req(
						owner_id_t owner,	
						handle_t handle
					 );
				virtual void execute(journal & );
			};

			struct cb_req : public internal_req
			{
				cb_req(
						owner_id_t owner,
						handle_t handle
					);

				owner_id_t const owner;
				handle_t const handle;

			};

			struct insert_cb_req : public cb_req
			{
				insert_cb_req(
						owner_id_t owner,
						handle_t handle
					);
				virtual void execute(journal &);
			};

			struct erase_cb_req : public cb_req
			{
				erase_cb_req(
						owner_id_t owner,
						handle_t handle
					);
				virtual void execute(journal &);
			};


			public:

			journal();
			~journal();

			//XXX replace with something reasonable
			typedef std::exception recovery_except;

			virtual void insert(
					owner_id_t owner,
					util::multi_buffer const & buf,
					insert_cb & cb
					) throw();

			virtual void erase(
					owner_id_t owner,
					handle_t handle,
					erase_cb & erase
					) throw();

			// should be called before any insert or erase
			virtual void recover(
					recovery_dispatcher & dispatcher
					) throw(recovery_dispatcher);

			public:
			typedef boost::shared_ptr<internal_req> req_ptr;//pomyslec nad requestem ktory nie bylby z najwyzszej klasy
			typedef boost::shared_ptr<sync_req> sync_req_ptr;
			typedef boost::shared_ptr<cb_req> cb_req_ptr;

			private:

			friend class in_mem_journal_thread;
			friend class sync_journal_thread;
			friend class journal_callback_worker;

			typedef std::map<std::pair<owner_id_t,handle_t>,
				boost::shared_ptr<insert_req> > insert_reqs_map_t;

			typedef std::map<std::pair<owner_id_t,handle_t>,
				boost::shared_ptr<erase_req> > erase_reqs_map_t;

			typedef util::worker_pool<cb_req_ptr> cb_workers_t;

			util::sync_queue<req_ptr> reqs;
			util::sync_queue<sync_req_ptr> sync_reqs;

			insert_reqs_map_t insert_reqs;
			erase_reqs_map_t erase_reqs;

			cb_workers_t cb_workers;			
		};

		class journal_thread : public util::virtual_dest
		{
			public:
				journal_thread(journal & j) : j(j)
				{
				}
			protected:
				journal & j;
		};

		class journal_single_thread : public journal_thread
		{
			public:
				journal_single_thread(journal & j)
					: journal_thread(j)
				{
				}
				virtual void operator()() const = 0;
		};	

		class in_mem_journal_thread : public journal_single_thread
		{
			public:
				in_mem_journal_thread( journal & j);
				virtual void operator()() const;
		};

		class sync_journal_thread : public journal_single_thread
		{
			public:
				sync_journal_thread( journal & j);
				virtual void operator()() const;
		};

		class journal_callback_worker : public journal_thread,
			public util::worker<journal::cb_req_ptr>
		{
			public:
				journal_callback_worker( journal &j) :
					journal_thread(j),
					util::worker<journal::cb_req_ptr>(j.cb_workers)
				{
				}

				virtual void handle(journal::cb_req_ptr const &t) const
				{
					t->execute(this->j);
				}
		};

		class journal_callback_worker_factory
			: public util::worker_factory<journal::cb_req_ptr>
		{
			public:
				journal_callback_worker_factory(journal &j) : j(j)
			{
			}

				virtual worker_ptr create_worker(
						util::worker_pool<journal::cb_req_ptr> & pool
						) const
				{
					return new boost::thread(journal_callback_worker(this->j));
				}
			private:
				journal & j;
		};

	} // namespace journal 
} // namespace coherent

#endif /* JOURNAL_H_5316 */

