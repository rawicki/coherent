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

#include <cstring>

#include <boost/lexical_cast.hpp>

#include <debug/asserts.h>
#include <util/file.h>

namespace coherent {
namespace util {

using namespace std;
using namespace boost;

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
	this->open_internal(flags, true, mode);
}

void file::open(int flags)
{
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
	if (this->fd == -1)
		throw io_exception(*this, errno, "open");
}

void file::close()
{
	d_assert(this->is_open(), "file \"" << this->path << "\" not open");
	int err = ::close(this->fd);
	if (err)
		throw io_exception(*this, errno, "close");
	this->fd = NOT_OPEN;
}

} // namespace util
} // namespace coherent

