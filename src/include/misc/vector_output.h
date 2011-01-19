/*
 * (C) Copyright 2010 Tomasz Zolnowski
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

#ifndef MISC_VECTOROUTPUT_H
#define MISC_VECTOROUTPUT_H

#include <iostream>
#include <vector>

namespace coherent {
namespace misc {


template <typename T>
std::ostream& operator<< (std::ostream& os, const std::vector<T>& vec)
{
    os << "[";
    for (typename std::vector<T>::const_iterator it=vec.begin() ; it!=vec.end(); ++it)
    {
        if (it!=vec.begin()) {
            os << ", ";
        }
        os << *it;
    }
    os << "]";
    return os;
}


} // namespace misc
} // namespace coherent

#endif /* MISC_VECTOROUTPUT_H */
