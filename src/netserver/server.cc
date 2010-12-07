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
#include <boost/bind.hpp>

#include "server.h"
#include "connection.h"
#include "echo.h"

namespace coherent
{
    namespace netserver
    {
        server::server()
        {
            // TODO: Refactor
            // Create socket.
            this->sock = socket(PF_INET, SOCK_STREAM, 0);
            if(sock < 0)
            {
                ::std::cerr << "opening stream socket";
                exit(EXIT_FAILURE);
            }

            sockaddr_in socket_server;
            // Name socket using wildcards.
            // It is an Internet address.
            socket_server.sin_family = AF_INET;
            // One computer can have several network addresses.
            // Let all addresses be useable for this socket.
            socket_server.sin_addr.s_addr = htonl(INADDR_ANY);
            // Choose any of valid port numbers.
            socket_server.sin_port = 0;
            // Associate the address with the socket.

            if(bind (sock, (struct sockaddr *) &socket_server, sizeof(socket_server)) < 0)
            {
                ::std::cerr << "binding stream socket";
                exit(EXIT_FAILURE);
            }
            // Find out assigned port number and print it out.
            socklen_t length = sizeof(socket_server);
            if(getsockname(sock, (struct sockaddr *) &socket_server, &length) < 0)
            {
                ::std::cerr << "getting socket name";
                exit(EXIT_FAILURE);
            }
            ::std::cerr << "Socket port #" << static_cast<unsigned int>(ntohs(socket_server.sin_port)) << "\n";

            // Start accepting connections.
            if(listen(sock, QUEUE_LENGTH) < 0)
            {
                ::std::cerr << "listening stream socket";
                exit(EXIT_FAILURE);
            }
        }
        server::~server()
        {
        }
        void server::accept()
        {
            // TODO: Refactor
            while(true)
            {
                int msgsock = ::accept(sock, NULL, NULL);
                if(msgsock < 0)
                {
                    ::std::cerr << "accept";
                }
                else
                {
                    ::boost::shared_ptr<connection> c(new connection(msgsock));
                        //TODO: Make it intrusive_ptr!!
                    connections.push_back(c);
                    main_responder(*c);
                    receiver_threads.create_thread(
                            ::boost::bind(&receiver_thread, *c));
                }
            }
        }
    }
}

