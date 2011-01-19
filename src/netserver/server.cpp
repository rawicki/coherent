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


#include <iostream>
#include <exception>
#include <string>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include "connection.h"
#include "threadpool.h"
#include "server.h"


const size_t BUFFER_LENGTH = 100;


using ::boost::asio::ip::tcp;


void f(tcp::socket * socket)
{
    for(;;)
    {
        char buf[BUFFER_LENGTH];
        ::boost::system::error_code error;
        size_t length = socket->read_some(::boost::asio::buffer(buf), error);
        if(error == ::boost::asio::error::eof)
        {
            break;
        }
        else if(error)
        {
            throw ::boost::system::system_error(error);
        }
        ::boost::asio::write(* socket, ::boost::asio::buffer(buf, length));
    }
}

namespace coherent
{
namespace netserver
{

server::server(const int port_num, accept_callback_t accept_callback, const int workers_num)
  : io_service(),
    acceptor(io_service, tcp::endpoint(tcp::v4(), port_num)),
    accept_callback(accept_callback),
    connections(),
    workers_threads(workers_num)
{
}

server::~server()
{
}

void server::run()
{
    connections.push_back(new connection(* this));
    workers_threads.run();
    io_service.run();
}

void server::new_connection()
{
    connections.push_back(new connection(* this));
}

}  // namespace netserver
}  // namespace coherent
