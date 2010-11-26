#include <boost/bind.hpp>

#include "echo.h"

namespace coherent
{
    namespace netserver
    {
        void main_responder(Connection & conn)
        {
            conn.read(MAX_BYTES, ::boost::bind(echo_responder, conn, _1));
        }

        void echo_responder(Connection & conn, Data & data)
        {
            conn.write(data);
        }
    }
}

