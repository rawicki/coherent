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

#ifndef __COHERENT_NETSERVER_SERVER_H__
#define __COHERENT_NETSERVER_SERVER_H__


#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include "connection.h"
#include "threadpool.h"


namespace coherent
{
namespace netserver
{


class connection;


class server
{
public:
    typedef ::boost::function<void(connection *)> accept_callback_t;
public:
    //::boost::ptr_vector<connection> connections;

    //::boost::thread_group worker_threads;
    //::boost::thread_group receiver_threads;
    //::boost::thread_group sender_threads;
    ::boost::asio::io_service io_service;
    ::boost::asio::ip::tcp::acceptor acceptor;
    accept_callback_t accept_callback;
    ::boost::ptr_deque<connection> connections;
    thread_pool workers_threads;
public:
    server(const int port_num, accept_callback_t accept_callback, const int workers_num=4);
    ~server();
    void run();
    void new_connection();
};

}  // namespace netserver
}  // namespace coherent


#endif
