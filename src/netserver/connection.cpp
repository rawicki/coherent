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


#include <utility>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <util/multi_buffer.h>
#include "server.h"
#include "connection.h"


using ::boost::asio::ip::tcp;


namespace coherent
{
namespace netserver
{


connection::connection(server & s)
  : server_(s),
    socket(s.io_service),
    out_queue(socket)
{
    server_.acceptor.async_accept(socket,
            ::boost::bind(& connection::handle_accept, this, _1));
}

void connection::handle_accept(const ::boost::system::error_code & error)
{
    if(!error)
    {
        server_.accept_callback(this);
        server_.new_connection();
    }
}

connection::~connection()
{
}

void connection::read(
        size_t message_size,
        read_callback_t read_callback)
{
    // TODO: buffering
    // read_observer_t read_observer(message_size, read_callback);
    // read_observers.push(read_observer);
    ptr_buffer_t buffer_ptr = new byte_t[message_size];
    socket.async_read_some(::boost::asio::buffer(buffer_ptr, message_size),
            ::boost::bind(& connection::handle_read, this, read_callback, buffer_ptr, _1, _2));
}


void connection::handle_read(
    read_callback_t read_callback,
    ptr_buffer_t buffer,
    const ::boost::system::error_code & error,
    size_t bytes_transferred)
{
    if(!error)
    {
        ::boost::function<void()> task = ::boost::bind(read_callback, bytes_transferred, buffer);
        server_.workers_threads.schedule(task);
    }
    else
    {
        // TODO: LOG ERROR
    }
}


}  // namespace netserver
}  // namespace coherent
