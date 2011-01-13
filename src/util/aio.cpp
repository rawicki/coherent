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

#include <fcntl.h>
#include <unistd.h>
#include <util/aio.h>

#include <debug/asserts.h>

namespace coherent {
namespace util {

using namespace std;
using namespace boost;

//======= aio_worker_factory ===================================================

aio_worker_factory::worker_ptr aio_worker_factory::create_worker(
	worker_pool<aio_request_ptr> & pool
	) const
{
	return new thread(aio_worker(pool));
}

//======= requests =============================================================

aio_open_req::aio_open_req(
	callback & cb,
	std::string const & path,
	int flags,
	int mode
	) :
	cb(cb),
	path(path),
	flags(flags),
	mode(mode)
{
}

void aio_open_req::handle_in_worker(aio_worker const & worker) const
{
	worker.handle_exact(*this);
}

aio_pread_req::aio_pread_req(
	callback & cb,
	int fd,
	char * buf,
	off_t off,
	ssize_t size
	) :
	cb(cb),
	buf(buf),
	off(off),
	size(size),
	fd(fd)
{
}

void aio_pread_req::handle_in_worker(aio_worker const & worker) const
{
	worker.handle_exact(*this);
}

aio_preadv_req::aio_preadv_req(
	callback & cb,
	int fd,
	std::vector<iovec> & iovecs,
	off_t off
	) :
	cb(cb),
	off(off),
	fd(fd)
{
	this->iovecs.swap(iovecs);
}

void aio_preadv_req::handle_in_worker(aio_worker const & worker) const
{
	worker.handle_exact(*this);
}

aio_pwrite_req::aio_pwrite_req(
	callback & cb,
	int fd,
	char const * buf,
	off_t off,
	ssize_t size
	) :
	cb(cb),
	buf(buf),
	off(off),
	size(size),
	fd(fd)
{
}

void aio_pwrite_req::handle_in_worker(aio_worker const & worker) const
{
	worker.handle_exact(*this);
}

aio_pwritev_req::aio_pwritev_req(
	callback & cb,
	int fd,
	std::vector<iovec> & iovecs,
	off_t off
	) :
	cb(cb),
	off(off),
	fd(fd)
{
	this->iovecs.swap(iovecs);
}

void aio_pwritev_req::handle_in_worker(aio_worker const & worker) const
{
	worker.handle_exact(*this);
}

aio_close_req::aio_close_req(callback & cb, int fd) :
	cb(cb),
	fd(fd)
{
}

void aio_close_req::handle_in_worker(aio_worker const & worker) const
{
	worker.handle_exact(*this);
}

//======= aio_worker ===========================================================

aio_worker::aio_worker(worker_pool<aio_request_ptr> & pool) :
	worker<aio_request_ptr>(pool)
{
}

void aio_worker::handle(aio_request_ptr const & req) const
{
	req->handle_in_worker(*this);
}

void aio_worker::handle_exact(aio_open_req const & req) const
{
	int fd = open(req.path.c_str(), req.flags, req.mode);
	req.cb.open_completed(req, fd, errno);
}

void aio_worker::handle_exact(aio_pread_req const & req) const
{
	ssize_t res = pread(req.fd, req.buf, req.size, req.off);
	req.cb.pread_completed(req, res, errno);
}

void aio_worker::handle_exact(aio_preadv_req const & req) const
{
	ssize_t res = preadv(
		req.fd,
		&(req.iovecs[0]),
		req.iovecs.size(),
		req.off
		);
	req.cb.preadv_completed(req, res, errno);
}

void aio_worker::handle_exact(aio_pwrite_req const & req) const
{
	ssize_t res = pwrite(req.fd, req.buf, req.size, req.off);
	req.cb.pwrite_completed(req, res, errno);
}

void aio_worker::handle_exact(aio_pwritev_req const & req) const
{
	ssize_t res = pwritev(
		req.fd,
		&(req.iovecs[0]),
		req.iovecs.size(),
		req.off
		);
	req.cb.pwritev_completed(req, res, errno);
}

void aio_worker::handle_exact(aio_close_req const & req) const
{
	int res = close(req.fd);
	req.cb.close_completed(req, res, errno);
}


//======= aio_context ==========================================================

aio_context::aio_context(uint32_t num_workers) : workers()
{
	workers.start(num_workers, aio_worker_factory());
}

aio_context::~aio_context()
{
	workers.stop();
}

void aio_context::submit_request(aio_request_ptr req)
{
	this->workers.schedule_work(req);
}

//======= async_file ===========================================================

async_file::async_file(aio_context & ctx, std::string const path) :
	ctx(ctx),
	path(path),
	open_cb(NULL),
	close_cb(NULL),
	fd(NOT_OPEN)
{
}

void async_file::submit_open(
	open_callback & cb,
	int flags,
	int mode
	)
{
	d_assert(!this->is_open(), "can't open an already opened file");
	d_assert(
		this->open_cb == NULL,
		"can't open a file which is just being opened"
		);
	d_assert(this->close_cb == NULL, "waht?");
	aio_request_ptr req(new aio_open_req(*this, this->path, flags, mode));
	this->open_cb = & cb;
	this->ctx.submit_request(req);
}

void async_file::submit_pread(
	pread_callback & cb,
	char * buf,
	off_t off,
	ssize_t size
	)
{
	d_assert(this->is_open(), "can't pread from closed file");
	aio_request_ptr req(new aio_pread_req(cb, this->fd, buf, off, size));
	this->ctx.submit_request(req);
}

void async_file::submit_preadv(
	preadv_callback & cb,
	std::vector<iovec> & iovecs,
	off_t off
	)
{
	d_assert(this->is_open(), "can't preadv from closed file");
	aio_request_ptr req(new aio_preadv_req(cb, this->fd, iovecs, off));
	this->ctx.submit_request(req);
}

void async_file::submit_pwrite(
	pwrite_callback & cb,
	char const * buf,
	off_t off,
	ssize_t size
	)
{
	d_assert(this->is_open(), "can't pwrite to closed file");
	aio_request_ptr req(new aio_pwrite_req(cb, this->fd, buf, off, size));
	this->ctx.submit_request(req);
}

void async_file::submit_pwritev(
	pwritev_callback & cb,
	std::vector<iovec> & iovecs,
	off_t off
	)
{
	d_assert(this->is_open(), "can't pwritev to closed file");
	aio_request_ptr req(new aio_pwritev_req(cb, this->fd, iovecs, off));
	this->ctx.submit_request(req);
}

void async_file::submit_close(
	close_callback & cb
	)
{
	d_assert(this->is_open(), "can't close closed file");
	d_assert(
		this->close_cb == NULL,
		"can't close a file which is just being closed"
		);
	d_assert(this->open_cb == NULL, "waht?");
	aio_request_ptr req(new aio_close_req(*this, this->fd));
	this->close_cb = & cb;
	this->ctx.submit_request(req);
}

void async_file::open_completed(
	aio_open_req const & req,
	int fd,
	int err
	)
{
	d_assert(this->fd == NOT_OPEN, "what? fd=" << this->fd);
	d_assert(this->open_cb != NULL, "what?");
	if (fd >= 0) {
		this->fd = fd;
		err = 0; // just to be sure
	}
	open_callback & cb = *this->open_cb;
	this->open_cb = NULL;
	cb.open_completed(req, 0);
}

void async_file::close_completed(
	aio_close_req const & req,
	int res,
	int err
	)
{
	d_assert(this->fd != NOT_OPEN, "what?");
	d_assert(this->close_cb != NULL, "what?");
	if (res == 0)
		this->fd = NOT_OPEN;

	close_callback & cb = *this->close_cb;
	this->close_cb = NULL;
	cb.close_completed(req, res, err);
}


} // namespace util 
} // namespace coherent

