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
#include <exception>
#include "../server.h"
#include "random_data.h"


int main(const int argc, const char * const * const argv)
{
    int retval = 0;
    try
    {
        if(argc != 2)
        {
            ::std::clog << "Usage: random_data_server <port>\n" << ::std::flush;
            retval = 1;
        }
        else
        {
            int port_number = ::std::atoi(argv[1]);
            ::coherent::netserver::server s(port_number, ::coherent::netserver::random_data_acceptor);
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
