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

#ifndef LOG_H_5272
#define LOG_H_5272

#include <log4cxx/logger.h>

#define LOG(level, message) \
do { if (coherent::log::log_##level->isGreaterOrEqual(coherent::log::global_log_limit) ) {\
	char _log_pf_buf[131072]; \
	coherent::log::convert_pretty_function(__PRETTY_FUNCTION__, _log_pf_buf); \
	LOG4CXX_##level( \
		log4cxx::Logger::getLogger(_log_pf_buf), \
		static_cast<std::stringstream const &>(std::stringstream().flush() << message).str() \
	) \
} } while(0)

namespace coherent {
namespace log {

void convert_pretty_function(char const * s, char * const out);

void setup_logger_test(
	std::string const & log_path,
	log4cxx::LevelPtr const & log_level = log4cxx::Level::getDebug()
	);
void setup_logger_prod(std::string const & log_path);
void flush_logger();

extern log4cxx::LevelPtr global_log_limit; //XXX move to log.h
extern log4cxx::LevelPtr log_FATAL;
extern log4cxx::LevelPtr log_ERROR;
extern log4cxx::LevelPtr log_WARN;
extern log4cxx::LevelPtr log_INFO;
extern log4cxx::LevelPtr log_DEBUG;
extern log4cxx::LevelPtr log_TRACE;

} // namespace log
} // namespace coherent

#endif /* LOG_H_5272 */
