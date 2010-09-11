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

#ifndef CONFIG_H_2944
#define CONFIG_H_2944

#include <string>

namespace coherent {
namespace config {

std::string get_version_string();
std::string get_build_information();
void print_running_information();

}
}

#endif /* CONFIG_H_2944 */