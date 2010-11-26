#ifndef ECHO_H_1234
#define ECHO_H_1234

#include <queue>
#include <boost/function.hpp>

#include "connection.h"

namespace coherent
{
    namespace netserver
    {
        void main_responder(Connection & conn);
        void echo_responder(Connection & conn, Data & data);
    }
}

#endif

