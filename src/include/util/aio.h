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

#ifndef AIO_H_3410
#define AIO_H_3410

#include <sys/uio.h>

#include <boost/shared_ptr.hpp>

#include <util/worker_pool.h>
#include <debug/asserts.h>

namespace coherent {
namespace util {

class aio_worker;

class aio_request : public virtual_dest
{
public:
	virtual void handle_in_worker(aio_worker const & worker) const = 0;

};

class aio_open_req :
	public aio_request,
	private boost::noncopyable
{
public:
	struct callback : public virtual_dest
	{
		virtual void open_completed(
			aio_open_req const & req,
			int fd,
			int err
			) = 0;
	};

	aio_open_req(
		callback & cb,
		std::string const & path,
		int flags,
		int mode
		);

	virtual void handle_in_worker(aio_worker const & worker) const;

	callback & cb;
	std::string const path;
	int flags;
	int mode;
};

class aio_pread_req :
	public aio_request,
	private boost::noncopyable
{
public:
	struct callback : public virtual_dest
	{
		virtual void pread_completed(
			aio_pread_req const & req,
			ssize_t res,
			int err
			) = 0;
	};
	
	aio_pread_req(callback & cb, int fd, char * buf, off_t off, ssize_t size);

	virtual void handle_in_worker(aio_worker const & worker) const;

	callback & cb;
	char * buf;
	off_t off;
	ssize_t size;
	int fd;
};

class aio_preadv_req :
	public aio_request,
	private boost::noncopyable
{
public:
	struct callback : public virtual_dest
	{
		virtual void preadv_completed(
			aio_preadv_req const & req,
			ssize_t res,
			int err
			) = 0; 
	};

	//destroys the vector
	aio_preadv_req(
		callback & cb,
		int fd,
		std::vector<iovec> & iovecs,
		off_t off
		);

	virtual void handle_in_worker(aio_worker const & worker) const;

	callback & cb;
	std::vector<iovec> iovecs;
	off_t off;
	int fd;
};


class aio_pwrite_req :
	public aio_request,
	private boost::noncopyable
{
public:
	struct callback : public virtual_dest
	{
		virtual void pwrite_completed(
			aio_pwrite_req const & req,
			ssize_t res,
			int err
			) = 0;
	};

	aio_pwrite_req(callback & cb, int fd, char const * buf, off_t off, ssize_t size);

	virtual void handle_in_worker(aio_worker const & worker) const;

	callback & cb;
	char const * buf;
	off_t off;
	ssize_t size;
	int fd;
};

class aio_pwritev_req :
	public aio_request,
	private boost::noncopyable
{
public:
	struct callback : public virtual_dest
	{
		virtual void pwritev_completed(
			aio_pwritev_req const & req,
			ssize_t res,
			int err
			) = 0; 
	};

	//destroys the vector
	aio_pwritev_req(
		callback & cb,
		int fd,
		std::vector<iovec> & iovecs,
		off_t off
		);

	virtual void handle_in_worker(aio_worker const & worker) const;

	callback & cb;
	std::vector<iovec> iovecs;
	off_t off;
	int fd;
};

class aio_close_req :
	public aio_request,
	private boost::noncopyable
{
public:
	struct callback : public virtual_dest
	{
		virtual void close_completed(
			aio_close_req const & req,
			int res,
			int err
			) = 0;
	};

	aio_close_req(callback & cb, int fd);

	virtual void handle_in_worker(aio_worker const & worker) const;

	callback & cb;
	int fd;
};

typedef boost::shared_ptr<aio_request> aio_request_ptr;

class aio_worker : public worker<aio_request_ptr>
{
public:
	aio_worker(worker_pool<aio_request_ptr> & pool);

	void handle(aio_request_ptr const & req) const;
	void handle_exact(aio_open_req const & req) const;
	void handle_exact(aio_pread_req const & req) const;
	void handle_exact(aio_preadv_req const & req) const;
	void handle_exact(aio_pwrite_req const & req) const;
	void handle_exact(aio_pwritev_req const & req) const;
	void handle_exact(aio_close_req const & req) const;
};

class aio_worker_factory : public worker_factory<aio_request_ptr>
{
public:
	virtual worker_ptr create_worker(worker_pool<aio_request_ptr> & pool) const;
};

class aio_context
{
public:
	aio_context(uint32_t num_workers);
	~aio_context();
	void submit_request(aio_request_ptr req);

private:
	worker_pool<aio_request_ptr> workers;
};


class async_file :
	private aio_open_req::callback,
	private aio_close_req::callback
{
public:
	async_file(aio_context & ctx, std::string const path);

	struct open_callback : public virtual_dest
	{
		virtual void open_completed(
			aio_open_req const & req,
			int err
			) = 0;
	};

	typedef aio_pread_req::callback pread_callback;
	typedef aio_preadv_req::callback preadv_callback;
	typedef aio_pwrite_req::callback pwrite_callback;
	typedef aio_pwritev_req::callback pwritev_callback;

	struct close_callback : public virtual_dest
	{
		virtual void close_completed(
			aio_close_req const & req,
			int res,
		 	int err
			) = 0;
	};

	void submit_open(
		open_callback & cb,
		int flags,
		int mode
		);
	void submit_pread(
		pread_callback & cb,
		char * buf,
		off_t off,
		ssize_t size
		);
	//destroys the vector
	void submit_preadv(
		preadv_callback & cb,
		std::vector<iovec> & iovecs,
		off_t off
		);
	void submit_pwrite(
		pwrite_callback & cb,
		char const * buf,
		off_t off,
		ssize_t size
		);
	//destroys the vector
	void submit_pwritev(
		pwritev_callback & cb,
		std::vector<iovec> & iovecs,
		off_t off
		);
	void submit_close(
		close_callback & cb
		);

	inline bool is_open() const;

private:
	virtual void open_completed(
		aio_open_req const & req,
		int fd,
		int err
		);
	virtual void close_completed(
		aio_close_req const & req,
		int res,
		int err
		);

	enum {
		NOT_OPEN = -1
	};

	aio_context & ctx;
	std::string path;
	open_callback * open_cb;
	close_callback * close_cb;
	int fd;
};

//======= INLINE IMPLEMENTATION ================================================

bool async_file::is_open() const
{
	return this->fd != NOT_OPEN;
}

} // namespace util 
} // namespace coherent

#endif /* AIO_H_3410 */

