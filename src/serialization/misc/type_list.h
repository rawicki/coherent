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


namespace coherent {
namespace misc {

//basics:
struct list_head
{
};

template <typename This, typename Next>
struct list_elem
{
};


//useful list contstructors:
template <typename T1> struct make_list1 {
    typedef list_elem<T1, list_head> value;
};
template <typename T1, typename T2> struct make_list2 {
    typedef list_elem<T1, typename make_list1<T2>::value> value;
};
template <typename T1, typename T2, typename T3> struct make_list3 {
    typedef list_elem<T1, typename make_list2<T2, T3>::value> value;
};
template <typename T1, typename T2, typename T3, typename T4> struct make_list4 {
    typedef list_elem<T1, typename make_list3<T2, T3, T4>::value> value;
};
template <typename T1, typename T2, typename T3, typename T4, typename T5> struct make_list5 {
    typedef list_elem<T1, typename make_list4<T2, T3, T4, T5>::value> value;
};



//apply operator F
template <template <class> class F, typename List>
struct list_apply;

template <template <class> class F>
struct list_apply<F, list_head>
{
    typedef list_head value;
};

template <template <class> class F, typename This, typename Next>
struct list_apply<F, list_elem<This,Next> >
{
    typedef list_elem< F<This>, typename list_apply<F, Next>::value > value;
};


//apply operator Fn
template <typename F, typename List>
struct list_apply_fn;

template <typename F>
struct list_apply_fn<F, list_head>
{
    static void apply(F &) {}
};

template <typename F, typename This, typename Next>
struct list_apply_fn<F, list_elem<This, Next> >
{
    static void apply(F & f)
    {
        f.template apply<This>();
        list_apply_fn<F, Next>::apply(f);
    }
};


//join lists
template <typename L1, typename L2>
struct list_join;

template <typename L1_This, typename L1_Next, typename L2>
struct list_join< list_elem<L1_This, L1_Next>, L2>
{
    typedef list_elem< L1_This, typename list_join<L1_Next, L2>::value > value;
};

template <typename L2>
struct list_join<list_head, L2>
{
    typedef L2 value;
};


} // namespace misc
} // namespace coherent

#endif /* MISC_TYPELIST_H */
