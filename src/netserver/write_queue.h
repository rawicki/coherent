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


#ifndef __COHERENT_NETSERVER_WRITE_QUEUE_H__
#define __COHERENT_NETSERVER_WRITE_QUEUE_H__


#include <memory>
#include <deque>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "queue.h"


namespace coherent
{
namespace netserver
{


class write_queue
{
public:
    typedef write_queue self_t;
    typedef ::std::size_t size_type;
    typedef unsigned char byte_t;
private:
    typedef ::boost::asio::const_buffers_1 message_t;
    typedef queue<message_t, ::std::deque, no_wrapper, ::std::allocator> queue_t;
    typedef ::std::size_t messages_count_t;
    typedef ::boost::asio::ip::tcp::socket socket_t;
    typedef const ::boost::system::error_code & error_code_ref;
    typedef ::boost::function<void(error_code_ref, size_type)> write_handle_t;
    typedef ::boost::mutex mutex_t;
    typedef ::boost::unique_lock<mutex_t> unique_lock_t;
private:
    socket_t & socket;
    bool waits;
    queue_t messages_queue;
    size_type bytes_unconfirmed;
    messages_count_t messages_unconfirmed;
    mutex_t mutex;
public:
    write_queue(socket_t & socket);
    ~write_queue();
    template <typename memory_t>
    void write(const memory_t * memory, size_type message_size);
private:
    void handle_write(error_code_ref error, size_type bytes_transferred, message_t message);
    void asio_direct_send(message_t message);
};


template <typename memory_t>
void write_queue::write(const memory_t * memory, size_type message_size)
{
    message_t message = ::boost::asio::buffer(memory, message_size);
    unique_lock_t lock(mutex);
    bytes_unconfirmed += message_size * sizeof(memory_t);
    messages_unconfirmed += 1;
    if(waits)
    {
        lock.unlock();
        messages_queue.push(message);
    }
    else
    {
        waits = true;
        lock.unlock();
        asio_direct_send(message);
    }
}


}  // namespace netserver
}  // namespace coherent


#endif
