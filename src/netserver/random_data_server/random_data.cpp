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
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "../connection.h"
#include "../threadpool.h"
#include "random_data.h"

int int_from_trimmed_str(unsigned char* data)
{
    ::std::string size_str;
    size_str = (const char *)data;
    ::boost::trim(size_str);
    return ::boost::lexical_cast<int>(size_str);
}

// FIXME: That is global, because I don't want to allocate
// big chunk of memory every time a request comes.
// Probably we should enclose acceptor and responder in one class
// and put the array there.
unsigned char response_buffer[1024*1024*10];

namespace coherent
{
namespace netserver
{


const size_t MAX_BYTES = 42;


void random_data_acceptor(connection * conn)
{
    conn->read(MAX_BYTES, ::boost::bind(& random_data_responder, conn, _1, _2));
}

void random_data_responder(connection * conn, size_t bytes, connection::ptr_buffer_t data)
{
    try
    {
        size_t size = int_from_trimmed_str(data);
        for(size_t i = 0; i < size; i++)
        {
            // In fact, we don't need random data - we need any data of requested length;
            response_buffer[i] = '9';
        }
        response_buffer[size - 1] = '\n';
        response_buffer[size] = '\0';
        conn->write(response_buffer, size + 1);
    }
    catch(::boost::bad_lexical_cast &)
    {
        conn->write((unsigned char *)"That's not a number: '", 22);
        conn->write(data, bytes);
        conn->write((unsigned char *)"'\n", 3);
    }

    conn->close();
}


}  // namespace netserver
}  // namespace coherent
