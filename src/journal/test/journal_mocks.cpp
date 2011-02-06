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

#include <config/config.h>
#include <debug/asserts.h>
#include <log/log.h>
#include <util/misc.h>
#include <util/thread.h>
#include <journal/journal.h>
#include <journal/test/journal_mocks.h>

namespace coherent {
namespace journal {
namespace unittests {

using namespace std;
using namespace boost;
using namespace coherent::util;

//==== in_mem_journal ==========================================================

struct journal_worker : public worker<in_mem_journal::req_ptr>
{
public:
	journal_worker(in_mem_journal & journal) :
		worker<in_mem_journal::req_ptr>(journal.workers),
		journal(journal)
	{
	}

protected:
	virtual void handle(in_mem_journal::req_ptr const & t) const
	{
		t->execute(this->journal);
	}

private:
	in_mem_journal & journal;
};

struct journal_worker_factory : public worker_factory<in_mem_journal::req_ptr>
{
	journal_worker_factory(in_mem_journal & journal) : journal(journal)
	{
	}

	virtual worker_ptr create_worker(
		worker_pool<in_mem_journal::req_ptr> & pool
		) const
	{
		return new thread(journal_worker(this->journal));
	}
private:
	in_mem_journal & journal;
};

in_mem_journal::in_mem_journal()
{
	journal_worker_factory fact(*this);
	this->workers.start(1, fact);
}

in_mem_journal::~in_mem_journal()
{
	this->workers.stop();
}

void in_mem_journal::insert(
	owner_id_t owner,
	util::multi_buffer const & buf,
	insert_cb & cb
	) throw()
{
	req_ptr req = req_ptr(
		new insert_req(
			owner,
			buf,
			cb
			)
		);
	this->workers.schedule_work(req);
}

void in_mem_journal::erase(
	owner_id_t owner,
	handle_t handle,
	erase_cb & cb 
	) throw()
{
	req_ptr req = req_ptr(
		new erase_req(
			owner,
			handle,
			cb
			)
		);
	this->workers.schedule_work(req);
}

void in_mem_journal::recover(
	recovery_dispatcher & dispatcher
	) throw(recovery_dispatcher)
{
	for (
		mapt_t::const_iterator it = this->contents.begin();
		it != this->contents.end();
		++it)
	{
		multi_buffer::buffer_list lst;
		lst.push_back(it->second);
		multi_buffer buf(lst, it->second->get_size(), 0);
		dispatcher.dispatch(
			it->first.first, //owner
			it->first.second, //handle
			buf
			);
	}
	
	this->contents.clear();
}

in_mem_journal::insert_req::insert_req(
	owner_id_t owner,
	util::multi_buffer const & buf,
	insert_cb & cb
	) :
	owner(owner),
	buf(buf),
	cb(cb)
{
}

void in_mem_journal::insert_req::execute(in_mem_journal & journal)
{
	handle_t handle;
	do {
		handle = random();
	} while (
		journal.contents.find(map_key_t(this->owner, handle))
			!= journal.contents.end()
		);
	buffer_ptr new_buf(new buffer(this->buf.get_size()));
	this->buf.read(new_buf->get_data(), this->buf.get_size(), 0);
	journal.contents.insert(
		make_pair(map_key_t(this->owner, handle), new_buf)
		);
	this->cb.insert_success(this->owner, this->buf, handle);
}

in_mem_journal::erase_req::erase_req(
	owner_id_t const owner,
	handle_t const handle,
	erase_cb & cb
	) :
	owner(owner),
	handle(handle),
	cb(cb)
{
}

void in_mem_journal::erase_req::execute(in_mem_journal & journal)
{
	bool erased = journal.contents.erase(map_key_t(this->owner, this->handle));
	r_assert(
		erased,
		"Entry (" << this->owner << "," << this->handle <<
		" does not exist in journal"
		);
	this->cb.erase_success(this->owner, this->handle);
}

//======== sync_journal_wrapper ================================================

sync_journal_wrapper::sync_journal_wrapper(journal & impl) : impl(impl)
{
}

journal::handle_t sync_journal_wrapper::insert(
	owner_id_t owner,
	util::multi_buffer const & buf
	)
{
	struct dummy_cb : public journal::insert_cb
	{
		dummy_cb(
			completion & cmpl,
			owner_id_t owner,
			multi_buffer const & buf
			) :
			cmpl(cmpl),
			owner(owner),
			buf(buf)
		{
		}

		virtual void insert_success(
			owner_id_t owner,
			util::multi_buffer const & buf,
			handle_t handle
			) throw()
		{
			r_assert(
				this->owner == owner,
				"mismatch: " << this->owner << " " << owner
				);
			r_assert(
				&this->buf == &buf,
				"mismatch: " << reinterpret_cast<void const *>(&this->buf) <<
				" " << reinterpret_cast<void const *>(&buf)
				);
			this->handle = handle;
		}

		virtual void insert_failure(
			owner_id_t owner,
			util::multi_buffer const & buf,
			int err
			) throw()
		{
			r_assert(
				this->owner == owner,
				"mismatch: " << this->owner << " " << owner
				);
			r_assert(
				&this->buf == &buf,
				"mismatch: " << reinterpret_cast<void const *>(&this->buf) <<
				" " << reinterpret_cast<void const *>(&buf)
				);
			r_assert(
				false,
				"unexpected failure for " << this->owner << " " <<
					reinterpret_cast<void const *>(&buf) << " " << err
				);
		}

		completion & cmpl;
		owner_id_t owner;
		multi_buffer const & buf;
		handle_t handle;
	};

	completion cmpl;
	dummy_cb cb(cmpl, owner, buf);
	this->impl.insert(owner, buf, cb);
	cmpl.wait();

	return cb.handle;
}

void sync_journal_wrapper::erase(
	owner_id_t owner,
	handle_t handle
	)
{
	struct dummy_cb : public journal::erase_cb
	{
		dummy_cb(
			completion & cmpl,
			owner_id_t owner,
			handle_t handle
			) :
			cmpl(cmpl),
			owner(owner),
			handle(handle)
		{
		}

		virtual void erase_success(
			owner_id_t owner,
			handle_t handle
			) throw()
		{
			r_assert(
				this->owner == owner,
				"mismatch: " << this->owner << " " << owner
				);
			r_assert(
				this->handle == handle,
				"mismatch: " << this->handle << " " << handle
				);
		}

		virtual void erase_failure(
			owner_id_t owner,
			handle_t handle,
			int err
			) throw()
		{
			r_assert(
				this->owner == owner,
				"mismatch: " << this->owner << " " << owner
				);
			r_assert(
				this->handle == handle,
				"mismatch: " << this->handle << " " << handle
				);
			r_assert(
				false,
				"unexpected failure for " << this->owner << " " <<
					this->handle << " " << err
				);
		}

		completion & cmpl;
		owner_id_t owner;
		handle_t handle;
	};

	completion cmpl;
	dummy_cb cb(cmpl, owner, handle);
	this->impl.erase(owner, handle, cb);
	cmpl.wait();
}

void sync_journal_wrapper::recover(
	recovery_dispatcher & dispatcher
	)
{
	this->impl.recover(dispatcher);
}

} // namespace unittests
} // namespace journal
} // namespace coherent

