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


#include <boost/bind.hpp>
#include <boost/asio/buffer.hpp>
#include "write_queue.h"


namespace coherent
{
namespace netserver
{


write_queue::write_queue(socket_t & socket)
  : socket(socket),
    waits(false),
    messages_queue(),
    bytes_unconfirmed(0),
    messages_unconfirmed(0)
{
}


write_queue::~write_queue()
{
}


void write_queue::handle_write(error_code_ref error, size_type bytes_transferred, message_t message, write_handler_t handler)
{
    unique_lock_t lock(mutex);
    messages_unconfirmed -= 1;
    bytes_unconfirmed -= bytes_transferred;
    size_type message_size = ::boost::asio::detail::buffer_size_helper(message);
    if(message_size != bytes_transferred)
    {
        lock.unlock();
        const void * offset_message = static_cast<const byte_t *>(::boost::asio::detail::buffer_cast_helper(message)) + bytes_transferred;
        // FIXME: add the bytes_transferred that was transferred already
        asio_direct_send(::boost::asio::buffer(offset_message, message_size - bytes_transferred), handler);
    }
    else
    {
        if(!messages_queue.empty())
        {
            lock.unlock();
            message_observer_t message = messages_queue.pop();
            asio_direct_send(message.first, message.second);
        }
        else
        {
            waits = false;
        }
    }
}


void write_queue::asio_direct_send(message_t message, write_handler_t handler)
{
    socket.async_write_some(message, wrap_handler(message, handler));
}


write_queue::write_handler_t write_queue::wrap_handler(message_t message, write_handler_t handler)
{
    return ::boost::bind(& write_queue::handle_write, this, _1, _2, message, handler);
}


}  // namespace netserver
}  // namespace coherent
