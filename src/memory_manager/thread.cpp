/*
 * (C) Copyright 2010 Xilexio
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

#include <stdlib.h>
#include <debug/asserts.h>
#include "thread.h"

namespace coherent
{
namespace memory_manager
{

pthread_key_t tls_key;

void memory_thread_clean(void* tls)
{
    delete reinterpret_cast<tls_content*> (tls);
}

void tls_init()
{
    r_assert(!pthread_key_create(&tls_key, memory_thread_clean));
}

void tls_clean()
{
    r_assert(!pthread_key_delete(tls_key));
}

void memory_thread_init_if_needed()
{
    tls_content* tlsc = tls();

    if (!tlsc)
    {
	tlsc = new tls_content;
	r_assert(!pthread_setspecific(tls_key, tlsc));
    }
}

}
}