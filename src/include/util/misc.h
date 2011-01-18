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

#ifndef MISC_H_2345
#define MISC_H_2345

#include <cassert>

#define stringify(s) stringify_helper(s)
#define stringify_helper(s) #s

#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#ifdef VALGRIND
#define VALGRIND_SLOWDOWN 1000
#else
#define VALGRIND_SLOWDOWN 1
#endif

namespace coherent {
namespace util {

template <class T>
bool is_pow2(T v)
{
	while (v != 0)
	{
		if ((v & 1) == 1)
			return v == 1;
		v = v >> 1;
	}
	return false;
}

template <class T, class A>
inline T align_down(T v, A alignment)
{
	//can't use d_assert here, because the file is included by asserts.h
	assert(is_pow2(alignment));
	return v - (v & (alignment - 1));
}

template <class T, class A>
inline T align_up(T v, A alignment)
{
	T res = align_down(v, alignment);
	return (res < v) ? (res + alignment) : res;
}

class virtual_dest
{
public:
	virtual ~virtual_dest() {}
};

} // namespace util
} // namespace coherent

#endif /* MISC_H_2345 */
