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


#ifndef __COHERENT_NETSERVER_QUEUE_H__
#define __COHERENT_NETSERVER_QUEUE_H__


#include <memory>
#include <deque>
#include <boost/function.hpp>
#include <boost/thread.hpp>


namespace coherent
{
namespace netserver
{


template<typename element_t>
class simple_ptr_wrapper
{
public:
    typedef element_t * ptr_t;
};

template<typename element_t>
class no_wrapper
{
public:
    typedef element_t ptr_t;
};


// Synchronized queue. Stores pointers to `element_t` type elements
// with producer-consumer synchronization on `pop` and `push`.
// Exception safe.
template <
    typename element_t,
    template <typename, typename> class container_template = ::std::deque,
    template <typename> class ptr_wrapper_template = simple_ptr_wrapper,
    template <typename> class allocator_template = ::std::allocator
    >
class queue
{
public:
    typedef typename ptr_wrapper_template<element_t>::ptr_t element_ptr_t;
    typedef allocator_template<element_ptr_t> element_ptr_allocator_t;
    typedef container_template<element_ptr_t, element_ptr_allocator_t> container_t;
private:
    typedef ::boost::mutex mutex_t;
    typedef ::boost::condition_variable condition_t;
    typedef ::boost::unique_lock<mutex_t> unique_lock_t;
    typedef ::boost::lock_guard<mutex_t> lock_guard_t;
private:
    condition_t condition;
    mutex_t mutex;
    container_t container;
public:
    queue();
    ~queue();
    element_ptr_t pop();
    void push(element_ptr_t element_ptr);
};


template <
    typename element_t,
    template <typename, typename> class container_template,
    template <typename> class ptr_wrapper_template,
    template <typename> class allocator_template
    >
queue<element_t, container_template, ptr_wrapper_template, allocator_template>::queue()
  : condition(),
    mutex(),
    container()
{
}


template <
    typename element_t,
    template <typename, typename> class container_template,
    template <typename> class ptr_wrapper_template,
    template <typename> class allocator_template
    >
queue<element_t, container_template, ptr_wrapper_template, allocator_template>::~queue()
{
}


template <
    typename element_t,
    template <typename, typename> class container_template,
    template <typename> class ptr_wrapper_template,
    template <typename> class allocator_template
    >
typename queue<element_t, container_template, ptr_wrapper_template, allocator_template>::element_ptr_t queue<element_t, container_template, ptr_wrapper_template, allocator_template>::pop()
{
    unique_lock_t lock(mutex);
    while(container.empty())
    {
        condition.wait(lock);
    }
    element_ptr_t element_ptr = container.front();
    container.pop_front();
    return element_ptr;
}


template <
    typename element_t,
    template <typename, typename> class container_template,
    template <typename> class ptr_wrapper_template,
    template <typename> class allocator_template
    >
void queue<element_t, container_template, ptr_wrapper_template, allocator_template>::push(element_ptr_t element_ptr)
{
    {
        lock_guard_t lock(mutex);
        container.push_back(element_ptr);
    }
    condition.notify_one();
}


}  // namespace netserver
}  // namespace coherent


#endif
