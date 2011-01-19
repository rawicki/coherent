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


#ifndef __COHERENT_NETSERVER_CONNECTION_H__
#define __COHERENT_NETSERVER_CONNECTION_H__


#include <utility>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include "server.h"
#include "write_queue.h"


using ::boost::asio::ip::tcp;


namespace coherent
{
namespace netserver
{


class server;


class connection
{
public:
    typedef unsigned char byte_t;
    typedef byte_t * ptr_buffer_t;
    typedef ::std::size_t size_type;
    typedef ::boost::function<void(size_t, ptr_buffer_t)> read_callback_t;
    typedef ::std::pair<size_t, read_callback_t> read_observer_t;
    typedef ptr_buffer_t message_t;
    typedef const ::boost::system::error_code & error_code_ref;
    typedef ::boost::function<void(error_code_ref, size_type)> write_callback_t;
private:
    server & server_;
    tcp::socket socket;
    write_queue out_queue;
public:
    connection(server & s);
    ~connection();
    void read(size_t message_size, read_callback_t callback);
    template <typename memory_t>
    void write(const memory_t * message, ::std::size_t message_size, write_callback_t callback);
    void close();
private:
    void handle_read(
            read_callback_t read_callback,
            ptr_buffer_t buffer,
            const ::boost::system::error_code & error,
            size_t bytes_transferred);
    void handle_accept(error_code_ref error);
};


template <typename memory_t>
void connection::write(const memory_t * message, ::std::size_t message_size, write_callback_t callback)
{
    out_queue.write(message, message_size, callback);
}


}  // namespace netserver
}  // namespace coherent


#endif
