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


#ifndef __COHERENT_NETSERVER_RANDOM_DATA_H__
#define __COHERENT_NETSERVER_RANDOM_DATA_H__


#include "../connection.h"


namespace coherent
{
namespace netserver
{


void random_data_acceptor(connection * connection);
void random_data_responder(connection * connection, size_t bytes, connection::ptr_buffer_t data);


}  // namespace netserver
}  // namespace coherent


#endif
