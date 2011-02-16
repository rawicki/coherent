/*
 * (C) Copyright 2010 Cezary Bartoszuk, Michał Stachurski, Rafał Rawicki
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


#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#include "callback.h"


namespace coherent
{
namespace netserver
{

void i_do_nothing()
{
}

void i_can_callback_on_write(const ::boost::system::error_code & error_code, ::std::size_t size)
{
}

const ::boost::function<void()> EMPTY_CALLBACK = & i_do_nothing;

const ::boost::function<void(const ::boost::system::error_code &, ::std::size_t)> EMPTY_WRITE_CALLBACK = i_can_callback_on_write;

}  // namespace netserver
}  // namespace coherent
