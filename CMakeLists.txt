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

cmake_minimum_required (VERSION 2.6)

project (Coherent)
enable_testing()

#path to addtional modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeScripts")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Content addressable, history enabled DB")
set(CPACK_PACKAGE_VENDOR "Marek Dopiera")
set(CPACK_PACKAGE_CONTACT "Marek Dopiera")


set(COMMON_FLAGS " -D_FILE_OFFSET_BITS=64 -rdynamic") #linux needs that to have 64bit off_t
set(WARNING_FLAGS " -Wall -Wunused-function -Wwrite-strings -Wformat -Wformat-security -Wparentheses -Wsequence-point -Wno-system-headers -Werror -Winit-self")
set(CFLAGS_DEBUG " -g -O0 -fstack-protector-all -D_GLIBXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC")
set(CFLAGS_REL " -O2 -DNDEBUG -Wuninitialized")

if (USER_COVERAGE)
	set(CFLAGS_COVERAGE " -fprofile-arcs -ftest-coverage")
else(USER_COVERAGE)
	set(CFLAGS_COVERAGE "")
endif (USER_COVERAGE)

if (VALGRIND_TEST)
	find_program(VALGRIND_BIN "valgrind")
	message(STATUS "Found valgrind: ${VALGRIND_BIN}")
	set(COMMON_FLAGS "${COMMON_FLAGS} -DVALGRIND")
	if (NOT VALGRIND_BIN)
		message(FATAL_ERROR "Valgrind not found")
	endif(NOT VALGRIND_BIN)
	set(TEST_SUFFIX "${VALGRIND_BIN}" "--leak-check=full" "--error-exitcode=13" "--track-origins=yes" "--suppressions=${CMAKE_SOURCE_DIR}/valgrind.supp" "--log-file=valgrind.log" "--")
else (VALGRIND_TEST)
	set(TEST_SUFFIX)
endif(VALGRIND_TEST)
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
message(STATUS "Selected CXXFLAGS: ${EFFECTIVE_CXX_FLAGS}")

find_package(Boost COMPONENTS program_options regex filesystem REQUIRED)
if (Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
	set(CORELIBS ${CORELIBS} ${Boost_LIBRARIES})
else (Boost_FOUND)
	message(FATAL_ERROR "Boost library not found")
endif (Boost_FOUND)

find_package(Log4Cxx REQUIRED)
if (Log4cxx_FOUND)
	include_directories(SYSTEM ${LOG4CXX_INCLUDE_DIR})
	set(CORELIBS ${CORELIBS} ${LOG4CXX_LIBRARIES})
else(Log4cxx_FOUND)
	message(FATAL_ERROR "Log4CXX library not found")
endif(Log4cxx_FOUND)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(TEST_BIN_DIR "test")
set(PROJECT_SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(TOP_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(TOP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src)
