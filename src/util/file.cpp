/*
 * (C) Copyright 2010 Marek Dopiera
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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

#include <cstring>

#include <boost/lexical_cast.hpp>

#include <debug/asserts.h>
#include <util/file.h>
#include <util/misc.h>

namespace coherent {
namespace util {

using namespace std;
using namespace boost;

//==== global helper ===========================================================


struct global_helper
{
	global_helper() : page_size(::sysconf(_SC_PAGESIZE))
	{
	}

	uint32_t const page_size;
};

static global_helper const globals;


//==== io_exception ============================================================


io_exception::io_exception(file const & file, std::string const & msg) :
	message(
		string("file \"") + file.path + "\" (fd: " +
		lexical_cast<string>(file.fd) + "): " + msg
		)
{
}

io_exception::io_exception(file const & file, int err, std::string const & op) :
	message(
		string("file \"") + file.path + "\" (fd: " +
		lexical_cast<string>(file.fd) + "): " + ::strerror(err) + " (" +
		lexical_cast<string>(err) + ")"
		)
{
}

const char* io_exception::what() const throw()
{
	return this->message.c_str();
}

io_exception::~io_exception()
{
}

//==== file ====================================================================


file::file(string const & path) :
	path(path),
	fd(NOT_OPEN)
{
}

auto_ptr<file> file::create(string const & path, int flags, int mode)
{
	auto_ptr<file> res(new file(path));
	res->create(flags, mode);
	return res;
}

std::auto_ptr<file> file::open(string const & path, int flags)
{
	auto_ptr<file> res(new file(path));
	res->open(flags);
	return res;
}

file::~file()
{
	d_assert(
		!this->is_open(),
		"destroying an open file: \"" << this->path << "\" fd: " << this->fd
		);
}

void file::create(int flags, int mode)
{
	LOG(
		DEBUG,
		"create path=\"" << this->path << "\" flags=" <<
		flags << " mode=" << oct << mode
		);
	this->open_internal(flags, true, mode);
}

void file::open(int flags)
{
	LOG(
		DEBUG,
		"open path=\"" << this->path << "\" flags=" << flags
		);
	this->open_internal(flags, false);
}

void file::open_internal(int flags, bool create, int mode)
{
	d_assert(
		(flags & (O_EXCL | O_CREAT)) == 0,
		"inapropriate flags: "<< flags
		);
	d_assert(!this->is_open(), "file already open");
	this->fd = ::open(
		this->path.c_str(),
		flags | (create ? (O_CREAT | O_EXCL) : 0),
		mode
		);
	if (this->fd == -1) {
		io_exception ex(*this, errno, "open");
		LOG(DEBUG, "open failed: " << ex.what());
		throw ex;
	}
	LOG(DEBUG, "open succeeded path=\"" << this->path << "\" fd=" << this->fd);
	
}

void file::close()
{
	LOG(
		DEBUG,
		"close fd=" << this->fd << " path=\"" << this->path << "\""
		);
	d_assert(this->is_open(), "file \"" << this->path << "\" not open");
	int err = ::close(this->fd);
	if (err)
		throw io_exception(*this, errno, "close");
	this->fd = NOT_OPEN;
}

file::multi_buffer_ptr file::read(uint32_t size, uint64_t offset)
{
	LOG(
		DEBUG,
		"read fd=" << this->fd << " path=\"" << this->path << "\" off=" <<
		offset << " size=" << size
		);
	d_assert(this->is_open(), "file \"" << this->path << "\" not open");
	if (size == 0)
	{
		multi_buffer::buffer_list list;
		return multi_buffer_ptr(new multi_buffer(list, 0, 0));
	}

	//The buffers should not be too small in order to be sure that the kernel
	//merges them, but on the other hand they shouldn't be too big not to stress
	//the allocator (fragmentation). I've arbitrarilly chosen 512Kb.
	uint32_t const single_buf_size = 512 * 1024;

	//if we're already dividing the read into smaller chunks we can as well
	//align it (e.g. O_DIRECT requires alignment to 512b and there are some bugs
	//which make that not enough - let's be wasteful and align it to one page)
	uint32_t const alignment = globals.page_size;

	uint32_t const aligned_off = align_down(offset, alignment);
	uint32_t const shift = offset - aligned_off;
	uint32_t const aligned_end = align_up(offset + size, alignment);
	uint32_t const aligned_size = aligned_end - aligned_off;

	multi_buffer::buffer_list bufs;
	uint32_t const num_bufs =
		((aligned_end - aligned_off) % single_buf_size == 0)
		? ((aligned_end - aligned_off) / single_buf_size)
		: ((aligned_end - aligned_off) / single_buf_size + 1);

	LOG(
		TRACE,
		"num_bufs=" << num_bufs << " aligned_size=" << aligned_size <<
		" aligned_off=" << aligned_off << " aligned_end=" << aligned_end
		);
	struct iovec vecs[num_bufs];

	uint64_t cur_off;
	uint32_t i;
	for (
		i = 0, cur_off = aligned_off;
		cur_off < aligned_end;
		cur_off += single_buf_size, ++i
		)
	{
		d_assert(i < num_bufs, "i=" << i << " num_bufs=" << num_bufs);
		uint32_t const buf_len = (cur_off + single_buf_size >= aligned_end)
			? (aligned_end - cur_off)
			: single_buf_size;
		multi_buffer::buffer_ptr buf(new buffer(buf_len, alignment));
		bufs.push_back(buf);
		vecs[i].iov_base = buf->get_data();
		vecs[i].iov_len = buf_len;
		LOG(TRACE, "allocated buffer " << cur_off << "-" << cur_off + buf_len);
	}
	ssize_t err;
	do {
		LOG(
			TRACE,
			"preadv(" << this->fd << ", (" << aligned_size << "), " << num_bufs
			<< ", " << aligned_off
			);
		err = preadv(this->fd, vecs, num_bufs, aligned_off);
		LOG(TRACE, "preadv returned " << err);
	} while (err == -1 && errno == EINTR);

	if (
		err != static_cast<ssize_t>(aligned_size)
		&& (err < 0 || err < static_cast<ssize_t>(size + shift))
		)
	{
		if (err >= 0)
			throw io_exception(
				*this,
				string("short read: ") + lexical_cast<string>(err) + " "
				+ lexical_cast<string>(aligned_size)
				);
		else
			throw io_exception(*this, errno, "preadv");
	}
	return multi_buffer_ptr(
		new multi_buffer(bufs, min(static_cast<ssize_t>(size), err), shift)
		);
}

void file::write(multi_buffer const & buf, uint64_t offset)
{
	LOG(
		DEBUG,
		"write fd=" << this->fd << " path=\"" << this->path << "\" off=" <<
		offset << " size=" << buf.get_size() 
		);
}

} // namespace util
} // namespace coherent

