#ifndef CONNECTION_H_1234
#define CONNECTION_H_1234

#include <queue>
#include <boost/function.hpp>

namespace coherent
{
    namespace netserver
    {
        struct Data
        {
            public:
                const size_t length;
                // TODO: smart pointer
                const void * const raw_data;
            public:
                Data(size_t length, const void * raw_data);
                ~Data();
        };

        void i_do_nothing();

        const ::boost::function<void()> EMPTY_CALLBACK = &i_do_nothing;

        class Connection
        {
            public:
                typedef int time_delta_t;
                typedef unsigned char byte_t;
                typedef ::boost::function<void(Data)> callback_t;
            public:
                static const int TIMEOUT_INFTY = -1;
            private:
                struct Observer
                {
                    public:
                        size_t length;
                        callback_t callback;
                        time_t timeout;
                        ::boost::function<void()> timeout_callback;
                    public:
                        Observer(size_t length, callback_t callback, time_t timeout, ::boost::function<void()> timeout_callback);
                        ~Observer();
                };
                struct Message
                {
                    public:
                        Data data;
                        time_t timeout;
                        ::boost::function<void()> timeout_callback;
                    public:
                        Message(Data data, time_t timeout, ::boost::function<void()> timeout_callback);
                        ~Message();
                };
                int fd;
                ::std::queue<Observer> read_observers;
                ::std::queue<Message> outgoing_messages;
                byte_t * read_buffer;
                size_t read_buffer_filled_size;
            public:
                Connection(int fd);
                ~Connection();
                void read(size_t message_length,
                        callback_t callback,
                        time_delta_t time_delta=TIMEOUT_INFTY,
                        ::boost::function<void()> timeout_callback=EMPTY_CALLBACK);
                void write(Data data,
                        time_delta_t time_delta=TIMEOUT_INFTY,
                        ::boost::function<void()> timeout_callback=EMPTY_CALLBACK);
                time_t from_now(time_delta_t time_delta);

            friend void receiver_thread(Connection & conn);
        };

        const size_t MAX_BYTES = 20;

        void receiver_thread(Connection & conn);
    };
};

#endif
