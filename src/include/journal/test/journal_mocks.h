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

#ifndef JOURNAL_MOCKS_15914
#define JOURNAL_MOCKS_15914

#include <map>
#include <boost/shared_ptr.hpp>

#include <util/worker_pool.h>
#include <journal/journal.h>

namespace coherent {
namespace journal {
namespace unittests {

class journal_worker;
class journal_worker_factory;

class in_mem_journal : public journal
{
public:
	in_mem_journal();
	~in_mem_journal();

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

private:
	struct internal_req : public util::virtual_dest
	{
		virtual void execute(in_mem_journal & journal) = 0;
	};

	struct insert_req : public internal_req
	{
		insert_req(
			owner_id_t owner,
			util::multi_buffer const & buf,
			insert_cb & cb
			);
		virtual void execute(in_mem_journal & journal);

		owner_id_t const owner;
		util::multi_buffer const & buf;
		insert_cb & cb;
	};

	struct erase_req : public internal_req
	{
		erase_req(
			owner_id_t const owner,
			handle_t const handle,
			erase_cb & cb
			);
		virtual void execute(in_mem_journal & journal);

		owner_id_t const owner;
		handle_t const handle;
		erase_cb & cb;
	};

public:
	typedef boost::shared_ptr<internal_req> req_ptr;
private:
	typedef std::pair<owner_id_t, handle_t> map_key_t;
	typedef boost::shared_ptr<util::buffer> buffer_ptr;
	typedef std::map<map_key_t, buffer_ptr> mapt_t;
	typedef util::worker_pool<req_ptr> workers_t;

	friend class journal_worker;
	friend class journal_worker_factory;

	mapt_t contents;
	workers_t workers;

};

//does not have any error handling
class sync_journal_wrapper
{
public:
	typedef journal::owner_id_t owner_id_t;
	typedef journal::handle_t handle_t;
	typedef journal::recovery_dispatcher recovery_dispatcher;

	sync_journal_wrapper(journal & journal);

	handle_t insert(
		owner_id_t owner,
		util::multi_buffer const & buf
		);

	void erase(
		owner_id_t owner,
		handle_t handle
		);

	void recover(
		recovery_dispatcher & dispatcher
		);

private:
	journal & impl;
};

} // namespace unittests
} // namespace journal 
} // namespace coherent

#endif /* JOURNAL_MOCKS_15914 */

