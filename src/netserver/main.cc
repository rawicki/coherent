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
