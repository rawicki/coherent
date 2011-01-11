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
#include "connection.h"
#include "echo.h"
#include "threadpool.h"


namespace coherent
{
namespace netserver
{


const size_t MAX_BYTES = 42;


void echo_acceptor(connection * conn)
{
    conn->read(MAX_BYTES, ::boost::bind(& echo_responder, conn, _1, _2));
}

void t1(connection * conn, join_point::shared_ptr_t join_p)
{
    ::boost::this_thread::sleep(::boost::posix_time::milliseconds(5000));
    conn->write(9, (unsigned char *)"t1 done\n");
    join_p->join();
}

void t2(connection * conn)
{
    conn->write(16, (unsigned char *)"2 tasks joined\n");
}

void echo_responder(connection * conn, size_t bytes, connection::ptr_buffer_t data)
{
    conn->write(bytes, data);
    conn->read(MAX_BYTES, ::boost::bind(& echo_responder, conn, _1, _2));

    join_point::shared_ptr_t join_p = join_point::create(2, ::boost::bind(& t2, conn));

    defer(::boost::bind(& t1, conn, join_p));
    defer(::boost::bind(& t1, conn, join_p));

}


}  // namespace netserver
}  // namespace coherent
