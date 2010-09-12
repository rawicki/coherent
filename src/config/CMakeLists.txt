# (C) Copyright 2010 Marek Dopiera
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
	config.cpp
	)
#set_property(SOURCE config.cpp APPEND PROPERTY DEPENDS genver)
add_library(config ${SOURCES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(
	genver "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/gen_version.sh" "${CMAKE_CURRENT_BINARY_DIR}/version.h"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/dump_cfg.sh" "${CMAKE_CURRENT_BINARY_DIR}/compile.h" "${EFFECTIVE_CXX_FLAGS}" "${EFFECTIVE_C_FLAGS}"
	COMMAND "touch" "config.cpp" #couldn't find a better way to force rebuilding
	)
add_dependencies(config genver)
