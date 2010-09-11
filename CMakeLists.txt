# (C) Copyright 2010 Marek Dopiera
# 
# This file is part of CherentDB.
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
# License along with Foobar. If not, see
# http://www.gnu.org/licenses/.

#Parts of code taken from LoXiM.

cmake_minimum_required (VERSION 2.6)

project (Coherent)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Content addressable, history enabled DB")
set(CPACK_PACKAGE_VENDOR "Marek Dopiera")
set(CPACK_PACKAGE_CONTACT "Marek Dopiera")


set(COMMON_FLAGS " -D_FILE_OFFSET_BITS=64") #linux needs that to have 64bit off_t
set(WARNING_FLAGS " -Wall -Wunused-function -Wwrite-strings -Wformat -Wformat-security -Wparentheses -Wsequence-point -Wno-system-headers -Werror -Winit-self")
set(CFLAGS_DEBUG " -g -O0 -fstack-protector-all -D_GLIBXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC")
set(CFLAGS_REL " -O2 -DNDEBUG -Wuninitialized")

if (USER_COVERAGE)
	set(CFLAGS_COVERAGE " -fprofile-arcs -ftest-coverage")
else(USER_COVERAGE)
	set(CFLAGS_COVERAGE "")
endif (USER_COVERAGE)

set(CMAKE_CXX_FLAGS_DEBUG " ${COMMON_FLAGS} ${WARNING_FLAGS} ${CFLAGS_DEBUG} ${CFLAGS_COVERAGE}")
set(CMAKE_C_FLAGS_DEBUG " ${COMMON_FLAGS} ${WARNING_FLAGS} ${CFLAGS_DEBUG} ${CFLAGS_COVERAGE}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG)


set(CMAKE_CXX_FLAGS_RELEASE " ${COMMON_FLAGS} ${WARNING_FLAGS} ${CFLAGS_REL}")
set(CMAKE_C_FLAGS_RELEASE " ${COMMON_FLAGS} ${WARNING_FLAGS} ${CFLAGS_REL}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE)


if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

#I have no idea why CMAKE_CXX_FLAGS is empty
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(EFFECTIVE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
	set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG}")
else(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(EFFECTIVE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
	set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS_RELEASE}")
endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

message(STATUS "Currnet build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Selected CXXFLAGS: ${CMAKE_CXX_FLAGS}")

find_package(Boost COMPONENTS program_options)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	set(CORELIBS ${CORELIBS} ${Boost_LIBRARIES})
else (Boost_FOUND)
	message(FATAL_ERROR "Boost library not found")
endif (Boost_FOUND)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(TOP_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
