#include <iostream>
#include <boost/bind.hpp>

#include "server.h"
#include "connection.h"
#include "echo.h"

namespace coherent
{
    namespace netserver
    {
        Server::Server()
        {
            // TODO: Refactor
            // Create socket.
            this->sock = socket(PF_INET, SOCK_STREAM, 0);
            if(sock < 0)
            {
                ::std::cerr << "opening stream socket";
                exit(EXIT_FAILURE);
            }

            sockaddr_in server;
            // Name socket using wildcards.
            // It is an Internet address.
            server.sin_family = AF_INET;
            // One computer can have several network addresses.
            // Let all addresses be useable for this socket.
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            // Choose any of valid port numbers.
            server.sin_port = 0;
            // Associate the address with the socket.

            if(bind (sock, (struct sockaddr *) &server, sizeof(server)) < 0)
            {
                ::std::cerr << "binding stream socket";
                exit(EXIT_FAILURE);
            }
            // Find out assigned port number and print it out.
            socklen_t length = sizeof(server);
            if(getsockname(sock, (struct sockaddr *) &server, &length) < 0)
            {
                ::std::cerr << "getting socket name";
                exit(EXIT_FAILURE);
            }
            ::std::cerr << "Socket port #" << static_cast<unsigned int>(ntohs(server.sin_port)) << "\n";

            // Start accepting connections.
            if(listen(sock, QUEUE_LENGTH) < 0)
            {
                ::std::cerr << "listening stream socket";
                exit(EXIT_FAILURE);
            }
        }
        Server::~Server()
        {
        }
        void Server::accept()
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
                    ::boost::shared_ptr<Connection> c(new Connection(msgsock));
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

