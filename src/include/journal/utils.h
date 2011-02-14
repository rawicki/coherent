/*
 * (C) Copyright 2011 Marek Dopiera
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

#ifndef UTILS_H_1423
#define UTILS_H_1423

#include <stdint.h>

#include <util/misc.h>
#include <map>

	template<class K,class V>
bool inline key_exists( const std::map<K,V> &m, const K &k)
{
	return m.find(k) != m.end();
}

	template<class K,class V>
const V & get_map_val( const std::map<K,V> &m, const K &k)
{
	typename std::map<K,V>::const_iterator it = m.find(k);
	r_assert(
			it != m.end(),
			"Key does not exist"//byc moze jakis toString by sie przydal
		);
	return it->second;
}


#endif /* UTILS_H_1423 */

