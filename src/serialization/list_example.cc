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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <cxxabi.h>
#include <inttypes.h>
#include "Misc/TypeList.h"


//template functor
template <typename T> struct Signed {};
template <typename T> struct Unsigned {};


//define list, length=6, some types with functors
typedef ListJoin<
            ListJoin<
                ListApply<
                    Unsigned,
                    makeList3<uint8_t, uint16_t, uint32_t>::value
                >::value,
                makeList1<std::string>::value
            >::value,
            ListApply<
                Signed,
                makeList2<int32_t, int64_t>::value
            >::value
    >::value List1;


//
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

//detailed type printer - it can be partially specialized (apply function can't be)
template <typename T>
struct TPrint
{
    static std::string type()
    {
        std::ostringstream os;
        os << "UnknownType[" << demangle<T>() << "]";
        return os.str();
    }
};

template <typename T>
struct TPrint<Signed<T> >
{
    static std::string type()
    {
        std::ostringstream os;
        os << "SignedType[" << demangle<T>() << ", " << sizeof(T) << " bytes]";
        return os.str();
    }
};

template <typename T>
struct TPrint<Unsigned<T> >
{
    static std::string type()
    {
        std::ostringstream os;
        os << "UnsignedType[" << demangle<T>() << ", " << sizeof(T) << " bytes]";
        return os.str();
    }
};


//main printer functor
struct Printer
{
    Printer() : count_(0)
    {
    }
    template <typename T>
    void apply()
    {
        std::cout << "Type " << (++count_) << " is " << TPrint<T>::type() << "." << std::endl;
    }
    uint32_t count_;
};

//bind printer with list
typedef ListApplyFn<Printer, List1> TypePrinter;


int main()
{
    Printer printer;

    TypePrinter::apply(printer);

    return 0;
}

