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

/*
 * This program creates a socket and then begins an infinite loop. Each time
 * through the loop it accepts a connection and prints out messages from it.
 * When the connection breaks, or a termination message comes through, the
 * program accepts a new connection.
 */


#include <iostream>
#include <exception>
#include "server.h"
#include "echo.h"


int main(const int argc, const char * const * const argv)
{
    int retval = 0;
    try
    {
        if(argc != 2)
        {
            ::std::clog << "Usage: netserver <port>\n" << ::std::flush;
            retval = 1;
        }
        else
        {
            int port_number = ::std::atoi(argv[1]);
            ::coherent::netserver::server s(port_number, ::coherent::netserver::echo_acceptor);
            s.run();
        }
    }
    catch(::std::exception & exn)
    {
        ::std::cerr << "Exception: " << exn.what() << "\n" << ::std::flush;
        retval = 1;
    }
    return retval;
}
