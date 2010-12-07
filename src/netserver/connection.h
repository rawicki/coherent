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

#ifndef CONNECTION_H_1234
#define CONNECTION_H_1234

#include <queue>
#include <boost/function.hpp>

namespace coherent
{
    namespace netserver
    {
        struct data_t
        {
            public:
                const size_t length;
                // TODO: smart pointer
                const void * const raw_data;
            public:
                data_t(size_t length, const void * raw_data);
                ~data_t();
        };

        void i_do_nothing();

        const ::boost::function<void()> EMPTY_CALLBACK = &i_do_nothing;

        class connection
        {
            public:
                typedef int time_delta_t;
                typedef unsigned char byte_t;
                typedef ::boost::function<void(data_t)> callback_t;
            public:
                static const int TIMEOUT_INFTY = -1;
            private:
                struct observer
                {
                    public:
                        size_t length;
                        callback_t callback;
                        time_t timeout;
                        ::boost::function<void()> timeout_callback;
                    public:
                        observer(size_t length, callback_t callback, time_t timeout, ::boost::function<void()> timeout_callback);
                        ~observer();
                };
                struct message
                {
                    public:
                        data_t data;
                        time_t timeout;
                        ::boost::function<void()> timeout_callback;
                    public:
                        message(data_t data, time_t timeout, ::boost::function<void()> timeout_callback);
                        ~message();
                };
                int fd;
                ::std::queue<observer> read_observers;
                ::std::queue<message> outgoing_messages;
                byte_t * read_buffer;
                size_t read_buffer_filled_size;
            public:
                connection(int fd);
                ~connection();
                void read(size_t message_length,
                        callback_t callback,
                        time_delta_t time_delta=TIMEOUT_INFTY,
                        ::boost::function<void()> timeout_callback=EMPTY_CALLBACK);
                void write(data_t data,
                        time_delta_t time_delta=TIMEOUT_INFTY,
                        ::boost::function<void()> timeout_callback=EMPTY_CALLBACK);
                time_t from_now(time_delta_t time_delta);

            friend void receiver_thread(connection & conn);
        };

        const size_t MAX_BYTES = 20;

        void receiver_thread(connection & conn);
    };
};

#endif
