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
void convert_pretty_function(char const * s, char * const out)
{
	size_t out_off = 0;
	size_t in_templ = 0; //nest in template
	size_t last_off = 0;
	assert(s);
	for (; s[0] != '('; ++s) {
		assert(s[0]);
		
		if (in_templ) {
			if (s[0] == '<')
			   ++in_templ;
			else if (s[0] == '>') {
				assert(in_templ);
				--in_templ;
			}
			continue;
		}
		if (s[0] == '<') {
			//check if it is not operatror< also start a template, but nex char
			//has to be '(' which will successfully break the loop
			++in_templ;
			continue;
		} else if (s[0] == ':') {
			last_off = out_off;
			out[out_off++] = '.';
			++s;
			assert(s[0] == ':');
			continue;
		}
		if (s[0] == ' ') { //return value separator
			out_off = 0;
			last_off = 0;
			continue;
		}
		out[out_off++] = s[0];
	}
	out[last_off] = 0;
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
	setup_logger(log_path, Level::getDebug(), Level::getTrace());
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
