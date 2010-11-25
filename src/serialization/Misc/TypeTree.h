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


//basics:
struct TreeLeaf
{
};

template <typename This, typename LeftNode, typename RightNode>
struct TreeNode
{
    typedef This Type;
};


//useful creators for simple trees
template <typename T>
struct CreateTreeNode
{
    typedef TreeNode<T, TreeLeaf, TreeLeaf> value;
};


#endif /* MISC_TYPETREE_H */
