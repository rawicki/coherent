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

#include <cassert>
#include <stdlib.h>
#include "thread.h"

namespace coherent {
namespace memory_manager {

void memoryThreadClean(void* tls) {
    delete static_cast<TLSContent*>(tls);
}

void tlsInit() {
    assert(!pthread_key_create(&tlsKey, memoryThreadClean));
}

void tlsClean() {
    assert(!pthread_key_delete(tlsKey));
}

void memoryThreadInitIfNeeded() {
    TLSContent* tlsContent = tls();

    if (!tlsContent) {
        tlsContent = new TLSContent;
        assert(!pthread_setspecific(tlsKey, tlsContent));
    }
}

}
}