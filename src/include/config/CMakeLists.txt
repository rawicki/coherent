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

include (CheckIncludeFiles)
include (CheckFunctionExists)
include (CheckCSourceCompiles)

CHECK_INCLUDE_FILES (execinfo.h HAVE_EXECINFO_H)
CHECK_FUNCTION_EXISTS (backtrace HAVE_BACKTRACE)
CHECK_C_SOURCE_COMPILES("int main()
{
    int l;
	__sync_bool_compare_and_swap(&l, 0, 1);
	return 0;
}" HAVE_SYNC_BOOL_COMPARE_AND_SWAP_INT)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/cmake_config.h)
