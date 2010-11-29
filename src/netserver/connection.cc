#include <ctime>
#include <iostream>
#include <unistd.h>
#include <boost/bind.hpp>
#include "connection.h"

#include <errno.h>

namespace coherent
{
    namespace netserver
    {
        void i_do_nothing()
        {
        }

        Data::Data(size_t length, const void * raw_data)
          : length(length),
            raw_data(raw_data)
        {
        }

        Data::~Data()
        {
        }

        Connection::Observer::Observer(size_t length, callback_t callback, time_t timeout, ::boost::function<void()> timeout_callback)
          : length(length),
            callback(callback),
            timeout(timeout),
            timeout_callback(timeout_callback)
        {
        }

        Connection::Observer::~Observer()
        {
        }

        Connection::Message::Message(Data data, time_t timeout, ::boost::function<void()> timeout_callback)
          : data(data),
            timeout(timeout),
            timeout_callback(timeout_callback)
        {
        }

        Connection::Message::~Message()
        {
        }

        Connection::Connection(int fd)
          : fd(fd),
            read_observers(),
            outgoing_messages(),
            read_buffer(NULL),
            read_buffer_filled_size(0)
        {
        }

        Connection::~Connection()
        {
            delete[] read_buffer;
            // read_buffer_filled_size = 0;
        }

        void Connection::read(size_t message_length,
                callback_t callback,
                time_delta_t time_delta,
                ::boost::function<void()> timeout_callback)
        {
            time_t time_deadline = from_now(time_delta);  // `time_delta` seconds from now on
            read_observers.push(Observer(message_length, callback, time_deadline, timeout_callback));
        }

        void Connection::write(Data data,
                time_delta_t time_delta,
                ::boost::function<void()> timeout_callback)
        {
            time_t time_deadline = from_now(time_delta);
            outgoing_messages.push(Message(data, time_deadline, timeout_callback));
        }

        time_t Connection::from_now(time_delta_t time_delta)
        {
            if(time_delta == TIMEOUT_INFTY)
            {
                return 0;
            }
            else
            {
                return time(NULL) + time_delta;
            }
        }

        void receiver_thread(Connection & conn)
        {
            // TODO: Refactor
            ssize_t rval;
            do
            {
                Connection::Observer obs = conn.read_observers.front();

                char * buf = new char[MAX_BYTES];
                // There's a memory leak here!!!

                memset(buf, 0, sizeof(buf));
                rval = read(conn.fd, buf, sizeof(buf));
                if(rval < 0)
                {
                    ::std::cerr << "Error reading stream message";
                }
                else if(rval == 0)
                {
                    ::std::clog << "Ending connection\n";
                }
                else
                {
                    ::std::clog << "-->" << (int)rval << "  " << buf << "\n";

                    obs.callback(Data(obs.length, buf));
                }
            }
            while(rval > 0);
            if(close(conn.fd) < 0)
            {
                ::std::cerr << "closing client socket";
            }
        }

        void writer_thread(Connection & conn)
        {
        }
    };
};
