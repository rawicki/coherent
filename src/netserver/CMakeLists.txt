# (C) Copyright 2010 Rafał Rawicki
#
# This file is part of CoherentDB.
#
# CoherentDB is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# CoherentDB is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with CoherentDB. If not, see
# http://www.gnu.org/licenses/.

set(SOURCES
    server.cpp
    main.cpp
    callback.cpp
    timeout.cpp
    connection.cpp
    write_queue.cpp
    threadpool.cpp
    echo.cpp
)

add_subdirectory(test)
add_subdirectory(random_data_server)
add_executable(netserver ${SOURCES})
target_link_libraries(netserver config buffercache log debug2 pthread ${CORELIBS})
