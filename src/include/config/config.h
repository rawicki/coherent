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

#ifndef CONFIG_H_2944
#define CONFIG_H_2944

#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include <boost/utility.hpp>

namespace coherent {
namespace config {

std::string get_version_string();
std::string get_build_information();
void print_running_information();

struct config_exception
{
	enum
	{
		NO_LINE = -1
	};

	int const line_no;
	std::string const descr;

	config_exception(std::string const & descr);
	config_exception(int line_no, std::string const & descr);

	std::string to_string();
};

class ini_config;

class config_section_base
{
protected:
	config_section_base(std::string const & sect, ini_config const & conf);
	void check_no_others(); //should be called in derived's ctor

	template<typename T>
	T get_value(std::string const & name, T const & def);
	template<typename T>
	T get_value(std::string const & name);
private:
	ini_config const & conf;
	std::string name;
	std::set<std::string> valid_names;
	std::set<std::string> present_names;
};

struct global_config : private boost::noncopyable
{
private:
	std::auto_ptr<ini_config> conf;

public:
	global_config(std::string const & file_name) throw(config_exception);

	struct buffer_cache_sect : public config_section_base, private boost::noncopyable
	{
		buffer_cache_sect(ini_config const & conf);

		uint64_t size;
		uint16_t syncer_sleep_time;
	};

	buffer_cache_sect buffer_cache;
};

} // namespace config
} // namespace coherent

#endif /* CONFIG_H_2944 */
