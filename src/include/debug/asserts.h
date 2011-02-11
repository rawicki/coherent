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
#include <cstdlib>
#include <iostream>

#include <util/misc.h>
#include <log/log.h>
#include <debug/stacktrace.h>

// counts number of arguments up to 3 (undefined behaviour for 3+ arguments)
#define count_varg2(_0, _1, _2, _3, n, ...) n
#define count_varg(...) count_varg2(0, ##__VA_ARGS__, 3, 2, 1, 0)

#define __assertion_impl2(c, m) \
	do {if (!(c)) { \
		LOG(FATAL, m); \
		coherent::log::flush_logger(); \
		LOG(FATAL, "aborting..."); \
		std::cerr << m << std::endl; \
		std::cerr.flush(); \
		std::cout.flush(); \
		::fflush(stdout); \
		::fflush(stderr); \
		::abort(); \
	} } while (0)

#define __assertion_impl(x, z, ...) \
	if (count_varg(__VA_ARGS__) == 1) \
		__assertion_impl2(x, z << " assertion failed (" << stringify(x) << ")" << \
			std::endl << "Message: " << "" __VA_ARGS__ << std::endl << \
				coherent::debug::stacktrace_as_string());  \
	else \
		__assertion_impl2(x, z << " assertion failed (" << stringify(x) << ")" << \
			std::endl << coherent::debug::stacktrace_as_string())

#ifndef NDEBUG
#define d_assert(x, ...) __assertion_impl(x, "Debug", ##__VA_ARGS__)
#else
#define d_assert(x, ...) do { if (false && (x)); } while (0)
#endif

#define r_assert(x, ...) __assertion_impl(x, "Release", ##__VA_ARGS__)

#endif /* ASSERTIONS_H_5673 */
