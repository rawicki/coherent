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

#ifndef MISC_TYPETREE_H
#define MISC_TYPETREE_H

namespace coherent {
namespace misc {


//basics:
struct tree_leaf
{
};

template <typename This, typename LeftNode, typename RightNode>
struct tree_node
{
};


//useful creators for simple trees
template <typename T>
struct create_tree_node
{
    typedef tree_node<T, tree_leaf, tree_leaf> value;
};


} // namespace misc
} // namespace coherent

#endif /* MISC_TYPETREE_H */
