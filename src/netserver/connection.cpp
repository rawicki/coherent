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

#include <ctime>
#include <iostream>
#include <unistd.h>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <buffercache/multi_buffer.h>
#include "connection.h"

#include <errno.h>

namespace coherent
{
    namespace netserver
    {
        void i_do_nothing()
        {
        }

        connection::observer::observer(size_t length, callback_t callback, time_t timeout, ::boost::function<void()> timeout_callback)
          : length(length),
            callback(callback),
            timeout(timeout),
            timeout_callback(timeout_callback)
        {
        }

        connection::observer::~observer()
        {
        }

        connection::message::message(::boost::shared_ptr<buffer> data, time_t timeout)
          : data(data),
            timeout(timeout)
        {
        }

        connection::message::~message()
        {
        }

        connection::connection(int fd, ::boost::function<void()> timeout_callback)
          : fd(fd),
            read_observers(),
            outgoing_messages(),
            read_buffer(NULL),
            read_buffer_filled_size(0),
            timeout_callback(timeout_callback)
        {
        }

        connection::~connection()
        {
            delete[] read_buffer;
            if(close(this->fd) < 0)
            {
                ::std::cerr << "Error closing client socket.\n";
            }
        }

        void connection::read(size_t message_length,
                callback_t callback,
                time_delta_t time_delta)
        {
            time_t time_deadline = from_now(time_delta);  // `time_delta` seconds from now on
            read_observers.push(observer(message_length, callback, time_deadline, timeout_callback));
        }

        void connection::write(::boost::shared_ptr<buffer> data,
                time_delta_t time_delta)
        {
            time_t time_deadline = from_now(time_delta);
            outgoing_messages.push(message(data, time_deadline));
        }

        time_t connection::from_now(time_delta_t time_delta)
        {
            if(time_delta == TIMEOUT_INFTY)
            {
                return 0;
            }
            else
            {
                return time(NULL) + time_delta;
            }
        }

        void receiver_thread(connection * conn)
        {
            // TODO: Refactor
            ssize_t rval;
            do
            {
                connection::observer obs = conn->read_observers.front();

                ::boost::shared_ptr<buffer> buf(new buffer(obs.length));
                rval = read(conn->fd, buf->get_data(), buf->get_size());

                if(rval < 0)
                {
                    ::std::cerr << "Error reading stream message";
                }
                else if(rval == 0)
                {
                    ::std::clog << "Ending connection\n";
                }
                else
                {
                    ::std::clog << "-->" << (int)rval << "  " << buf->get_data() << "\n";

                    obs.callback(buf);
                }
            }
            while(rval > 0);
        }

        void writer_thread(connection * conn)
        {
        }
    };
};
