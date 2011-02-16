# (C) Copyright 2010 Marek Dopiera
# 
# This file is part of CoherentDB.
# 
# CoherentDB is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# CoherentDB is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public
# License along with CoherentDB. If not, see
# http://www.gnu.org/licenses/.


include_directories("include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include") #used for generated includes
add_subdirectory(include)
add_subdirectory(config)
add_subdirectory(server)
add_subdirectory(log)
add_subdirectory(debug)
add_subdirectory(util)
#add_subdirectory(memory_manager) will be added later
add_subdirectory(buffercache)
add_subdirectory(netserver)
add_subdirectory(journal)
