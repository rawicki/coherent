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

#ifndef SERVER_H_1234
#define SERVER_H_1234

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace coherent
{
    namespace netserver
    {
        const int QUEUE_LENGTH = 5;
        const size_t BUFFER_SIZE = 1024;

        class Connection;

        class Server
        {
            private:
                int sock;

                ::std::vector< ::boost::shared_ptr<Connection> > connections;

                ::boost::thread_group receiver_threads;
                ::boost::thread_group sender_threads;
            public:
                Server();
                ~Server();
                void accept();
        };
    }
}

#endif

