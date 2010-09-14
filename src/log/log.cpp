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

#include <sstream>
#include <cassert>
#include <cstring>

#include <log/log.h>

#include <log4cxx/fileappender.h>
#include <log4cxx/patternlayout.h>

using namespace std;
using namespace log4cxx;

namespace coherent {
namespace log {

LevelPtr log_FATAL(Level::getFatal());
LevelPtr log_ERROR(Level::getError());
LevelPtr log_WARN(Level::getWarn());
LevelPtr log_INFO(Level::getInfo());
LevelPtr log_DEBUG(Level::getDebug());
LevelPtr log_TRACE(Level::getTrace());
LevelPtr global_log_limit;

//we want to convert pretty function to the format
//namespace1.namespace2.class1.class2
//XXX rewrite to C - this is probably very slow
string convert_pretty_function(string const & s)
{
	assert(s.size());
	size_t const space = s.find(' ');
	assert(space != std::string::npos);
	assert(space != 0);
	std::string no_ret(s.substr(space + 1));
	assert(no_ret.size());

	size_t const paren = no_ret.find('(');
	assert(paren != std::string::npos);
	assert(paren != 0);
	no_ret.resize(paren);

	string res;
	res.reserve(paren);
	size_t last_pos = 0;
	for (
		size_t new_pos = no_ret.find(':', last_pos);
		new_pos != string::npos;
		new_pos = no_ret.find(':', last_pos)
	)
	{
		assert(new_pos + 2 < paren);
		//I think that every : should be followed by a second one and it can't
		//be the last token
		assert(no_ret.at(new_pos + 1) == ':');
		if (!res.empty())
			res += '.';
		res += no_ret.substr(last_pos, new_pos - last_pos);
		last_pos = new_pos + 2;
	}
	return res;
}

static void setup_logger(string const & log_path, LevelPtr const & log_level,
		LevelPtr const &  global_log_limit_arg)
{
	LoggerPtr root_logger = Logger::getRootLogger();
	root_logger->setLevel(log_level);
	char buf[sizeof(LevelPtr)];

	LevelPtr gl2(global_log_limit_arg);
	memcpy(buf, &gl2, sizeof(global_log_limit_arg));
	memcpy(&gl2, &global_log_limit, sizeof(global_log_limit));
	memcpy(&global_log_limit, buf, sizeof(global_log_limit));
//	global_log_limit = global_log_limit_arg; <--- should be that way but log4cxx
//	is ugly ano compiler complains

	root_logger->addAppender(
		new FileAppender(
			new PatternLayout("%r %p %m [%c (%F:%L)]%n"),
			log_path,
			false,
			true,
			8192
		)
	);
}

void setup_logger_test(string const & log_path, LevelPtr const & log_level)
{
	setup_logger(log_path, log_level, Level::getTrace());
}

void setup_logger_prod(string const & log_path)
{
#ifndef NDEBUG
	setup_logger(log_path, Level::getInfo(), Level::getTrace());
#else
	setup_logger(log_path, Level::getInfo(), Level::getInfo());
#endif
}

void flush_logger()
{
	AppenderList apps = log4cxx::Logger::getRootLogger()->getAllAppenders();
	for (AppenderList::iterator i = apps.begin(); i != apps.end(); ++i) {
		FileAppender * app = dynamic_cast<FileAppender *>(&(**i));
		if (app)
			app->setImmediateFlush(true);
	}
}

} // namespace log
} // namespace coherent
