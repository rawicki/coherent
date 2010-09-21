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

#ifndef ASSERTIONS_H_5673
#define ASSERTIONS_H_5673

#include <cstdio>
#include <iostream>

#include <util/misc.h>
#include <log/log.h>
#include <debug/stacktrace.h>

//XXX flush logs
#define __assertion_impl2(c, m) \
	do {if (!(c)) { \
		LOG(FATAL, m); \
		coherent::log::flush_logger(); \
		LOG(FATAL, "aborting..."); \
		std::cerr << m << std::endl; \
		::fflush(stdout); \
		::abort(); \
	} } while (0)

#define __assertion_impl(x, y, z) \
	__assertion_impl2(x, z << " assertion failed (" << stringify(x) << ")" << \
		std::endl << "Message: " << y << std::endl << \
		   	coherent::debug::stacktrace_as_string())  \

#ifndef NDEBUG
#define d_assert(x, y) __assertion_impl(x, y, "Debug")
#else
#define d_assert(x, y) do { if (false && (x)); } while (0)
#endif

#define r_assert(x, y) __assertion_impl(x, y, "Release")

#endif /* ASSERTIONS_H_5673 */
