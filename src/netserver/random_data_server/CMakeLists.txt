#
# (C) Copyright 2010 Cezary Bartoszuk, Michał Stachurski, Rafał Rawicki
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
    ../server.cpp
    main.cpp
    ../callback.cpp
    ../timeout.cpp
    ../connection.cpp
    ../threadpool.cpp
    ../write_queue.cpp
    random_data.cpp
)

add_executable(random_data_server ${SOURCES})
target_link_libraries(random_data_server config buffercache log debug2 pthread ${CORELIBS})
