#ifndef SERVER_H_1234
#define SERVER_H_1234

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace coherent
{
    namespace netserver
    {
        const int QUEUE_LENGTH = 5;
        const size_t BUFFER_SIZE = 1024;

        class Connection;

        class Server
        {
            private:
                int sock;

                ::std::vector< ::boost::shared_ptr<Connection> > connections;

                ::boost::thread_group receiver_threads;
                ::boost::thread_group sender_threads;
            public:
                Server();
                ~Server();
                void accept();
        };
    }
}

#endif

