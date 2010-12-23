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

#ifndef FILE_22941
#define FILE_22941

#include <memory>
#include <string>

#include <boost/noncopyable.hpp>

namespace coherent {
namespace util {

class file;

class io_exception
{
public:
	io_exception(file const & file, std::string const & msg);
	io_exception(file const & file, int err, std::string const & op);

	virtual const char* what() const throw();

	virtual ~io_exception();
private:
	std::string message;
};

class file : private boost::noncopyable
{
public:
	file(std::string const & path);

	//O_CREAT and O_EXCL should not be passed
	static std::auto_ptr<file> create(
		std::string const & path,
		int flags,
		int mode
		);
	static std::auto_ptr<file> open(
		std::string const & path,
		int flags
		);
	~file();

	void create(int flags, int mode);
	void open(int flags);
	void close();

	inline bool is_open() const;

private:
	friend class io_exception;

	enum {
		NOT_OPEN = -1
	};

	void open_internal(int flags, bool create, int mode = 0);

	std::string path;
	int fd;

};

//======== inline implementation ===============================================

bool file::is_open() const { return this->fd != NOT_OPEN; }

} // namespace util
} // namespace coherent

#endif /* FILE_22941 */
