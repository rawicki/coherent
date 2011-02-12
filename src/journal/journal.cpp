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
	in_mem_thread = new boost::thread(in_mem_journal_thread(*this));
	sync_thread = new boost::thread(sync_journal_thread(*this));

	journal_callback_worker_factory factory(*this);
	this->cb_workers.start(2,factory);
}

journal::~journal()
{
        LOG(TRACE, "thread " << pthread_self() << " is ~journal");
	
	reqs.no_more_input();
	sync_reqs.no_more_input();

	in_mem_thread->join();
	delete in_mem_thread;

	sync_thread->join();
	delete sync_thread;

	this->cb_workers.stop();
}

void journal::insert(
			owner_id_t owner,
			multi_buffer const & buf,
			insert_cb & cb
			) throw()
{
	reqs.push(req_ptr(new insert_req(owner,buf,cb)));
	//cb.insert_success(owner,buf,random());
}

void journal::erase(
		owner_id_t owner,
		handle_t handle,
		erase_cb & cb
		) throw()
{
	reqs.push(req_ptr(new erase_req(owner,handle,cb)));
}

void journal::recover(
		recovery_dispatcher & dispatcher
		) throw(recovery_dispatcher)
{
}

//====== in_mem_journal_thread =================================================
in_mem_journal_thread::in_mem_journal_thread( journal & j)
	: journal_single_thread(j)
{
}

void in_mem_journal_thread::operator()() const
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

journal::insert_req::insert_req(
			owner_id_t owner,
			multi_buffer const & buf,
			insert_cb & cb
			) :
		owner(owner),
		buf(buf),
		cb(cb)
{
}


void journal::insert_req::execute(journal & j)
{
	handle_t handle;
	do {
		handle = random();
	} while( key_exists(j.insert_reqs,make_pair(this->owner,handle)));

	j.insert_reqs[make_pair(this->owner,handle)] = shared_from_this();

	j.sync_reqs.push(
		sync_req_ptr(
			new insert_sync_req(this->owner,handle,this->buf)));
}

journal::erase_req::erase_req(
		owner_id_t const owner,
		handle_t const handle,
		erase_cb & cb
	 ) :
	owner(owner),
	handle(handle),
	cb(cb)
{
}

void journal::erase_req::execute(journal & j)
{
	LOG(TRACE, "thread " << pthread_self() << " made a sync_req");

	j.erase_reqs[make_pair(this->owner,this->handle)] = shared_from_this();

	j.sync_reqs.push(
		sync_req_ptr(
			new erase_sync_req(this->owner,this->handle)));
}

//====== sync_journal_thread ===================================================
sync_journal_thread::sync_journal_thread( journal & j)
	: journal_single_thread(j)
{
//otworzyc plik
}

void sync_journal_thread::operator()() const
{
        LOG(TRACE, "thread " << pthread_self() << " has been started");
        boost::optional<journal::sync_req_ptr> res = j.sync_reqs.pop();
        while (res)
        {
		LOG(TRACE, "thread " << pthread_self() << " handle sync_req");
		res.get()->execute(j);
		res = j.sync_reqs.pop();
	}
        LOG(TRACE, "thread " << pthread_self() << " finishing");
}

//====== sync_reqs =============================================================

journal::sync_req::sync_req(
		owner_id_t owner,
		handle_t handle
		) : owner(owner), handle(handle)
{
}

journal::insert_sync_req::insert_sync_req(
		owner_id_t owner,
		handle_t handle,
		const multi_buffer & buf
		) : sync_req(owner,handle), buf(buf)
{
}

void journal::insert_sync_req::execute( journal & j)
{
	LOG(TRACE, "thread " << pthread_self() << " execute");
	j.cb_workers.schedule_work( cb_req_ptr( new insert_cb_req(this->owner, this->handle)));
}

journal::erase_sync_req::erase_sync_req(
		owner_id_t owner,
		handle_t handle
		) : sync_req(owner,handle)
{
}

void journal::erase_sync_req::execute( journal & j)
{
	LOG(TRACE, "thread " << pthread_self() << " execute");
	j.cb_workers.schedule_work( cb_req_ptr( new erase_cb_req(this->owner, this->handle)));
}

//====== cb_reqs ===============================================================

journal::cb_req::cb_req(
		owner_id_t owner,
		handle_t handle
		) : owner(owner), handle(handle)
{
}


journal::insert_cb_req::insert_cb_req(
		owner_id_t owner,
		handle_t handle
		) : cb_req( owner,handle)
{
}

journal::erase_cb_req::erase_cb_req(
		owner_id_t owner,
		handle_t handle
		) : cb_req( owner,handle)
{
}

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

