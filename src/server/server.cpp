/*
 * (C) Copyright 2010 Marek Dopiera
 * 
 * This file is part of CherentDB.
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
 * License along with Foobar. If not, see
 * http://www.gnu.org/licenses/.
 */

#include <cmath>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/math/distributions/normal.hpp>

#include <config/config.h>

using namespace std;
using namespace boost::program_options;

#define verOpt "version"
#define helpOpt "help"

namespace coherent {
namespace server {

using namespace config;

class user_args
{
public:
	user_args(int const argc, char * argv[]) :
		vm(this->gen_vm(argc, argv)),
		help(this->vm.count(helpOpt)),
		version(this->vm.count(verOpt))
	{
		if (!this->help && !this->version)
			this->usage_exit("", 1);
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
			(verOpt",v", "print version and exit")
			(helpOpt",h", "print this message and exit");
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


	
	variables_map const vm;

public:
	bool const help;
	bool const version;
};

} //namespace server
} //namespace coherent

int main(int const argc, char * argv[])
{
	using namespace std;
	cout << coherent::config::get_version_string() << endl << endl;
	cout << "Copyright (C) 2010 Marek Dopiera" << endl;
	cout << "This is free software; see the source for copying conditions.  There is NO" << endl;
	cout << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl << endl;
	coherent::server::user_args const u_args(argc, argv);
	if (u_args.version)
	{
		cout << coherent::config::get_build_information() << endl;
		coherent::config::print_running_information();
	}
	return 0;
}

