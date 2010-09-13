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

#include <cmath>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/math/distributions/normal.hpp>

#include <log/log.h>
#include <config/config.h>

using namespace std;
using namespace boost::program_options;

#define ver_opt "version"
#define help_opt "help"
#define log_opt "log_path"

namespace coherent {
namespace server {

using namespace config;

class user_args
{
public:
	user_args(int const argc, char * argv[]) :
		vm(this->gen_vm(argc, argv)),
		help(this->vm.count(help_opt)),
		version(this->vm.count(ver_opt)),
		log_path(this->extract_req_opt<string>(log_opt))
	{
		if (this->help)
			this->usage_exit("", 0);
	}

private:
	
	void usage_exit(string const & desc, int ret)
	{
		if (desc != "")
			cout << desc << endl;
		cout << gen_opt_desc() << endl;
		::exit(ret);
	}
	static options_description gen_opt_desc()
	{
		options_description desc("");
		desc.add_options()
			(ver_opt",v", "print version and exit")
			(help_opt",h", "print this message and exit")
			(log_opt",l", value<string>(), "log file path");
		return desc;
	}

	variables_map gen_vm(int const argc, char * argv[])
	{
		try {
			variables_map vm;
			store(parse_command_line(argc, argv, this->gen_opt_desc()), vm);
			notify(vm);
			return vm;
		} catch (exception & ex) {
			this->usage_exit(ex.what(), 1);
			//not reached, but the compiler complains :/
			return variables_map();
		}
	}

	template <class T>
	T extract_req_opt(string const & opt)
	{
		if (this->vm.count(opt) != 1) {
			if (!this->help && !this->version)
				this->usage_exit(string("Error: option ") + opt + " not specified", 1);
			else
				return T();
		}
		return this->vm[opt].as<T>();
	}

	variables_map const vm;

public:
	bool const help;
	bool const version;
	string const log_path;
};

string welcome_string()
{
	stringstream ss;
	ss << coherent::config::get_version_string() << endl << endl;
	ss << "Copyright (C) 2010 Marek Dopiera" << endl;
	ss << "This is free software; see the source for copying conditions.  There is NO" << endl;
	ss << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl << endl;
	return ss.str();
}

} //namespace server
} //namespace coherent

int main(int const argc, char * argv[])
{
	using namespace std;
	using namespace coherent::server;
	
	cout << welcome_string() << endl;

	user_args const u_args(argc, argv);

	if (u_args.version)
	{
		cout << coherent::config::get_build_information() << endl;
		coherent::config::print_running_information();
		return 0;
	}

	coherent::log::setup_logger_prod(u_args.log_path);

	LOG(INFO, welcome_string());

	LOG(INFO, "CoherentDB exiting.");
	return 0;
}

