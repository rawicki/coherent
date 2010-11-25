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

#ifndef MISC_TYPELIST_H
#define MISC_TYPELIST_H


//basics:
struct ListHead
{
};

template <typename This, typename Next>
struct ListElem
{
    typedef This Type;
};


//useful list contstructors:
template <typename T1> struct makeList1 {
    typedef ListElem<T1, ListHead> value;
};
template <typename T1, typename T2> struct makeList2 {
    typedef ListElem<T1, typename makeList1<T2>::value> value;
};
template <typename T1, typename T2, typename T3> struct makeList3 {
    typedef ListElem<T1, typename makeList2<T2, T3>::value> value;
};
template <typename T1, typename T2, typename T3, typename T4> struct makeList4 {
    typedef ListElem<T1, typename makeList3<T2, T3, T4>::value> value;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5> struct makeList5 {
    typedef ListElem<T1, typename makeList4<T2, T3, T4, T5>::value> value;
};


#endif /* MISC_TYPELIST_H */
