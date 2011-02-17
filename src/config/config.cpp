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

#include <ctype.h>
#include <fcntl.h>
#include <sys/param.h>

#include <sstream>
#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <functional>
#include <fstream>
#include <sstream>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include <config/config.h>
#include <config/cmake_config.h>
#include <debug/common.h>
#include <log/log.h>

#include <version.h>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace log4cxx;

using namespace coherent::log;
using namespace coherent::debug;

namespace coherent {
namespace config {


string get_version_string()
{
	string res = "CoherentDB";
#ifdef OFFICIAL_RELEASE
	res += " " OFFICIAL_RELEASE;
#else
	res += " unofficial release";
#ifdef GIT_HASH
#ifdef GIT_BRANCH
	res += " branch \"" GIT_BRANCH "\"";
#endif /*GIT_BANCH*/
	res += " commit hash: " GIT_HASH;
#else
	res += " (unknown hash)";
#endif /* GIT_HASH */
#endif /* OFFICIAL_RELEASE */
	return res;
}

string get_build_information()
{
	stringstream ss;
	ss << "Built for " BUILD_OS "(" BUILD_ARCH ") on " BUILD_TIME << endl << 
		"Full build sytem information:" << endl << FULL_UNAME << endl <<
		"Compiled with CXXFLAGS:" << COMPILED_CXX_FLAGS << endl << "CFLAGS:" << COMPILED_C_FLAGS;
	return ss.str();
}

void print_running_information()
{
	cout << "Running on:" << endl;
	int ret = ::system("uname -a");
	if (ret == -1)
		cout << "(unable to obtain)" << endl;
}

class ini_config
{
public:
	typedef std::string section;
	typedef std::vector<section> sections;
	typedef std::vector<std::string> attribute_names;

	ini_config(std::string const & file);
	sections get_sections() const;
	attribute_names get_attribute_names(section const & section) const;
	std::string get_value(section const & sect, std::string const & attr) const;

private:
	typedef std::pair<section, std::string> id; //section, attr
	typedef std::map<id, std::string> value_map;
	typedef value_map::iterator value_map_it;

	value_map values;
};


//================ ini_config implementation ===================================

struct is_not_blank
{
	bool operator()(char c) const
	{
		return !isblank(c);
	}
};

static bool line_is_white(string const & s)
{
	return find_if(s.begin(), s.end(), is_not_blank()) == s.end();
}

static string strip_comments(string const & s)
{
	size_t hash = s.find(';');
	string res;
	if (hash == string::npos)
		res = s;
	else
		res = s.substr(0, hash);
	if (line_is_white(res))
		return "";
	else
		return res;
}

static string get_attr(string const & attr_line)
{
	string tmp = attr_line;
	size_t pos = tmp.find('=');
	d_assert(pos != string::npos, "line \"" << tmp << "\" does not contain '=' sign!");
	tmp.resize(pos);
	stringstream ss(tmp);
	string res;
	ss >> res;
	return res;
}

static string get_val(string const & attr_line)
{
	size_t const pos = attr_line.find('=');
	d_assert(pos != string::npos, "line \"" << attr_line << "\" does not contain '=' sign!");
	d_assert(pos + 1 < attr_line.length(), "line \"" << attr_line <<
			"\" does not contain a value");
	string const res = attr_line.substr(pos + 1);
	d_assert(!line_is_white(res), "line \"" << attr_line <<
			"\" does not contain a value");
	return res;
}

static string get_sect_name(string const & s)
{
	size_t const begin = s.find('[');
	d_assert(begin != string::npos, "section name \"" << s << "\" does not contain '[' sign");
	size_t const end = s.find(']');
	d_assert(end != string::npos, "section name \"" << s << "\" does not contain ']' sign");
	d_assert(begin < end, "']' cannot be before '[' in secton \"" << s << "\"");
	string const & res_with_white = s.substr(begin + 1, end - begin - 1);
	d_assert(!line_is_white(res_with_white), "no section name in \"" << s << "\"");
	stringstream ss(res_with_white);
	string res;
	ss >> res;
	d_assert(!res.empty(), "no section name in \"" << s << "\"");
	return res;
}

ini_config::ini_config(string const & file)
{
	LOG(TRACE, "parsing file " << file);
	regex const reg_attr("[ \\t]*[a-zA-Z][a-zA-Z0-9_]*[ \\t]*=[ \\t]*[^ \\t=\\[\\[][^=\\[\\]]*");
	regex const reg_sect("[ \\t]*\\[[ \\t]*[a-zA-Z][a-zA-Z0-9_]*[ \\t]*\\][ \\t]*");

	section cur_sect;
	int line_no = 0;

	ifstream in_f(file.c_str());
	if (!in_f)
		throw config_exception(string("failed to open file \"") + file + "\"");
	while (!!in_f) {
		++line_no;
		string line;
		getline(in_f, line);
		line = strip_comments(line);
		if (line == "")
			continue;
		if (regex_match(line, reg_attr)) {
			if (cur_sect.empty())
				throw config_exception(line_no,
						"each attribute has to be in a section");
			string const & val = get_val(line);
			string const & attr = get_attr(line);

			if (!this->values.insert(make_pair(id(cur_sect, attr), val)).second)
				throw config_exception(line_no, "duplicate entry");

		} else if (regex_match(line, reg_sect)) {
			cur_sect = get_sect_name(line);
			value_map_it it = this->values.lower_bound(id(cur_sect, ""));
			if (it != this->values.end() && it->first.first == cur_sect)
				throw config_exception(line_no, "section redefinition");
			LOG(TRACE, "parsing section " << cur_sect);
		} else
			throw config_exception(line_no, "invalid line format");
	}

	LOG(INFO, "Config file dump follows:");
	for (value_map_it it = this->values.begin(); it != this->values.end(); ++it)
		LOG(INFO, it->first.first << ", " << it->first.second << " = \"" <<
				it->second << "\"");
	LOG(INFO, "Config file dump finished.");
}

string ini_config::get_value(section const & sect, string const & attr) const
{
	value_map::const_iterator const it = this->values.find(id(sect, attr));
	d_assert(it != this->values.end(), "referring to non-existent sect " << sect
			<< " attr " << attr);
	return it->second;
}

ini_config::attribute_names ini_config::get_attribute_names(section const & sect) const
{
	attribute_names res;
	for (
		value_map::const_iterator it = this->values.lower_bound(id(sect, ""));
		it != this->values.end() && it->first.first == sect;
		++it
	)
		res.push_back(it->first.second);
	return res;
}

//================= config_exception implementation ============================

config_exception::config_exception(
	std::string const & descr)
: 
	line_no(NO_LINE), descr(descr)
{
}

config_exception::config_exception(
	int line_no, std::string const & descr)
: 
	line_no(line_no), descr(descr)
{
}

string config_exception::to_string()
{
	if (line_no == NO_LINE)
		return this->descr;
	else
		return string("line ") + lexical_cast<string>(this->line_no) + ": " +
			this->descr;
}

//================= config_section_base implementation =========================

config_section_base::config_section_base(
	string const & sect,
	ini_config const & conf
) :
   	conf(conf), name(sect)
{
	ini_config::attribute_names const & names =
		this->conf.get_attribute_names(this->name);
	this->present_names.insert(names.begin(), names.end());
}

template<typename T>
T config_section_base::get_value(std::string const & name, T const & def)
{
	bool const insert_res = this->valid_names.insert(name).second;
	d_assert(insert_res, "Referring to attribute " << name << "in section " <<
			this->name << " twice");
	set<string>::const_iterator const i = this->present_names.find(name);
	if (i == this->present_names.end())
		return def;
	else {
		try {
			T const & res = lexical_cast<T>(this->conf.get_value(this->name, name));
			return res;
		} catch (bad_lexical_cast &) {
			throw config_exception(string("section ") + this->name
					+ ", attribute " + name + ": invalid value");
		}
	}
}

template<typename T>
T config_section_base::get_value(std::string const & name)
{
	bool const insert_res = this->valid_names.insert(name).second;
	d_assert(insert_res, "Referring to attribute " << name << "in section " <<
			this->name << " twice");
	set<string>::const_iterator const i = this->present_names.find(name);
	if (i == this->present_names.end())
		throw config_exception(string("section ") + this->name + ", attribute "
				+ name + ": required attribute not present");
	else {
		try {
			T const & res = lexical_cast<T>(this->conf.get_value(this->name, name));
			return res;
		} catch (bad_lexical_cast &) {
			throw config_exception(string("section ") + this->name
					+ ", attribute " + name + ": invalid value");
		}
	}
}

void config_section_base::check_no_others()
{
	for (
		set<string>::const_iterator it = this->present_names.begin();
		it != this->present_names.end();
		++it
	) {
		if (this->valid_names.find(*it) == this->valid_names.end())
			throw config_exception(string("section ") + this->name
					+ ", attribute " + *it + ": unknown attribute");
	}
}

//================= global_config implementation ===============================

global_config::global_config(string const & file_name) throw(config_exception) :
	conf(new ini_config(file_name)),
	buffer_cache(*this->conf),
        memory_manager(*this->conf)
{

}

//================= buffer_cache_sect implementation ===========================

global_config::buffer_cache_sect::buffer_cache_sect(ini_config const & conf) : 
	config_section_base("buffer_cache", conf),
	size(this->get_value<uint64_t>("size")),
	syncer_sleep_time(this->get_value<uint16_t>("syncer_sleep_time", 30))
{
	this->check_no_others();
}

//================= memory_manager_sect implementation =======================

global_config::memory_manager_sect::memory_manager_sect(ini_config const& conf) :
	config_section_base("memory_manager", conf),
	ram_limit_bytes(this->get_value<uint64_t>("ram_limit_bytes")),
	total_limit_bytes(this->get_value<uint64_t>("total_limit_bytes")),
	default_session_ram_limit_bytes(this->get_value<uint64_t>("default_session_ram_limit_bytes")),
	default_session_total_limit_bytes(this->get_value<uint64_t>("default_session_total_limit_bytes")),
	max_small_alloc_pages_1024(this->get_value<uint16_t>("max_small_alloc_pages_1024")),
	single_small_alloc_pages(this->get_value<uint16_t>("single_small_alloc_pages"))

{
	this->check_no_others();
}

global_config::~global_config()
{
}

//================= scoped_test_enabler implementation =========================

scoped_test_enabler::scoped_test_enabler(int argc,
		char const * const * const argv, LevelPtr def_log_level)
{
#define cerr_assert(cond, msg) do { if (!(cond)) { cerr << msg << endl; ::abort(); } ; } while (0)
	string const required_path = "/bin/test/"; //just for safety
	string const work_dir = "run";
	string const tests_dir = FOR_TESTS_BIN_DIR  "/" + work_dir;
	
	char full_path_raw[MAXPATHLEN];
	cerr_assert(realpath(argv[0], full_path_raw), "real_path failed with errno=" << errno);
	string const full_path = full_path_raw;

	cerr_assert(full_path.find(required_path) != string::npos,
			"This is bad, path=" << tests_dir);
	size_t pos = full_path.rfind('/');
	if (pos == string::npos)
		pos = 0;
	string const test_name = full_path.substr(pos);
	string const target_dir_name = tests_dir + "/" + test_name;
	this->working_dir = target_dir_name;

	if (argc != 2 || strcmp(argv[1], "ready")) {
		//XXX unix specific
		if (exists(target_dir_name)) {
			cerr_assert(is_directory(target_dir_name), "I wanted to create a working "
					" directory for test but it already exists and is not a directory: "
					<< target_dir_name);
			remove_all(target_dir_name);
		}
		create_directories(target_dir_name);
		int err = chdir(target_dir_name.c_str());
		cerr_assert(!err, "chdir to " << target_dir_name << " failed with code " << errno);
		
		int out_fd = open("stdout", O_WRONLY | O_CREAT | O_EXCL, 0644);
		cerr_assert(err != -1, "creating stdout file failed with errno=" << errno);
		int err_fd = open("stderr", O_WRONLY | O_CREAT | O_EXCL, 0644);
		cerr_assert(err != -1, "creating stderr file failed with errno=" << errno);
		err = dup2(out_fd, 1);
		cerr_assert(err != -1, "dup2 of stdout file failed with errno=" << errno);
		err = dup2(err_fd, 2);
		cerr_assert(err != -1, "dup2 of stderr file failed with errno=" << errno);
		err = close(out_fd);
		cerr_assert(err != -1, "close of stdout file failed with errno=" << errno);
		err = close(err_fd);
		cerr_assert(err != -1, "close of stderr file failed with errno=" << errno);

		vector<string> args;
		if (argc > 1) {
			for (int i = 1; i < argc; ++i)
				args.push_back(argv[i]);
		}
		args.push_back(full_path);
		args.push_back("ready");
		char const * new_argv[args.size()+1];
		for (size_t i = 0; i < args.size(); ++i)
			new_argv[i] = args[i].c_str();
		new_argv[args.size()] = 0;
		char const * const to_run = (argc > 1) ? argv[1] : full_path.c_str();
		execvp(to_run, const_cast<char * const *>(new_argv));
		cerr_assert(false, "execvp failed with errno=" << errno);
	}
	setup_logger_test(target_dir_name + "/logs", def_log_level);
	LOG(INFO, "Test " << test_name << " starts");

	set_terminate_handler();

	this->config = shared_ptr<global_config>(
			new global_config(FOR_TESTS_SRC_DIR "/doc/default.conf"));

}

scoped_test_enabler::~scoped_test_enabler()
{
	LOG(INFO, "Test finished successfully.");
}

string scoped_test_enabler::get_working_dir()
{
	return this->working_dir;
}

shared_ptr<global_config> scoped_test_enabler::get_config()
{
	return this->config;
}

} // namespace config
} // namespace coherent
