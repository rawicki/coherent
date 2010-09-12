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
#include <iostream>
#include <cstdlib>

#include "version.h"
#include <compile.h>
#include <config/config.h>

using namespace std;

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
		"Compiled with CXXFLAGS:" << COMPILED_CXXFLAGS << endl << "CFLAGS:" << COMPILED_CFLAGS;
	return ss.str();
}

void print_running_information()
{
	cout << "Running on:" << endl;
	int ret = system("uname -a");
	if (ret == -1)
		cout << "(unable to obtain)" << endl;
}

}
}

