/*
 * (C) Copyright 2010 Cezary Bartoszuk
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


#include <algorithm>
#include <log/log.h>
#include <debug/debug.h>
#include <debug/asserts.h>
#include <config/config.h>
#include "../queue.h"


namespace coherent
{
namespace netserver
{
namespace unittests
{


const int ELEMENTS_QUANTITY = 10000;


int next_natural()
{
    static int n = 0;
    return n++;
}


int it_should_behave_like_normal_queue()
{
    int * bigarray = new int[ELEMENTS_QUANTITY];
    ::std::generate(bigarray, bigarray + ELEMENTS_QUANTITY, next_natural);
    ::coherent::netserver::queue<int> queue;
    for(int i = 0; i < ELEMENTS_QUANTITY; ++i)
    {
        queue.push(bigarray + i);
    }
    for(int i = 0; i < ELEMENTS_QUANTITY; ++i)
    {
        int * ptr = queue.pop();
        r_assert(ptr == bigarray + i, "Pointers do not match.");
    }
    return 0;
}

int start_test(const int argc, const char * const * const argv)
{
    ::coherent::config::scoped_test_enabler test_setup(argc, argv);
    ::log4cxx::Logger::getLogger("coherent.netserver.unittests")->setLevel(coherent::log::log_TRACE);
    return it_should_behave_like_normal_queue();
}


}  // namespace unittests
}  // namespace netserver
}  // namespace coherent


int main(const int argc, const char * const * const argv)
{
    return ::coherent::netserver::unittests::start_test(argc, argv);
}
