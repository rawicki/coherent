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

#ifndef MISC_DEMANGLE_H
#define MISC_DEMANGLE_H

#include <cstdlib>
#include <string>
#include <typeinfo>
#include <cxxabi.h>


template <typename T>
inline std::string demangle()
{
    int ret = 0;
    std::string xtype_s;
    char * xtype = abi::__cxa_demangle(typeid(T).name(), NULL, NULL, &ret);
    if (ret==0) {
        xtype_s = xtype;
        free(xtype);
    }
    else {
        xtype_s = std::string("Failed[") + typeid(T).name() + "]";
    }
    return xtype_s;
}


#endif /*MISC_DEMANGLE_H */
