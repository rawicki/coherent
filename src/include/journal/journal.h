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

#ifndef JOURNAL_H_5316
#define JOURNAL_H_5316

#include <stdint.h>

#include <exception>

#include <boost/noncopyable.hpp>

#include <util/misc.h>
#include <util/multi_buffer.h>

namespace coherent {
namespace journal {


class journal :
	public util::virtual_dest,
	private boost::noncopyable
{
public:
	typedef uint32_t owner_id_t;
	typedef uint32_t handle_t;

	//XXX replace with something reasonable
	typedef std::exception recovery_except;

	struct insert_cb : public util::virtual_dest
	{
		virtual void insert_success(
			owner_id_t owner,
			util::multi_buffer & buf,
			handle_t handle
			) throw() = 0;
		virtual void insert_failure(
			owner_id_t owner,
			util::multi_buffer & buf,
			int err
			) throw() = 0;
	};

	struct erase_cb : public util::virtual_dest
	{
		virtual void erase_success(
			owner_id_t owner,
			handle_t handle
			) throw() = 0;
		virtual void erase_failure(
			owner_id_t owner,
			handle_t handle,
			int err
			) throw() = 0;
	};
	
	struct recovery_dispatcher : public util::virtual_dest
	{
		virtual void dispatch(
			owner_id_t owner,
			handle_t handle,
			util::multi_buffer const & buffer
		) throw() = 0;
	};

	virtual void insert(
		owner_id_t,
		util::multi_buffer const & buf,
		insert_cb & cb
		) throw() = 0;

	virtual void erase(
		owner_id_t,
		handle_t handle,
		erase_cb & erase
		) throw() = 0;

	// should be called before any insert or erase
	virtual void recover(
		recovery_dispatcher & dispatcher
		) throw(recovery_dispatcher) = 0;

};

} // namespace journal 
} // namespace coherent

#endif /* JOURNAL_H_5316 */

