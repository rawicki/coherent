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
   This program creates a socket and then begins an infinite loop. Each time
   through the loop it accepts a connection and prints out messages from it.
   When the connection breaks, or a termination message comes through, the
   program accepts a new connection.
   */

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "server.h"

int main()
{
    coherent::netserver::Server s;

    s.accept();

    return 0;
}
