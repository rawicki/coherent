# (C) Copyright 2011 Marek Dopiera
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_BIN_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${TEST_BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${TEST_BIN_DIR})

add_library(journal_mock journal_mocks.cpp)

add_executable(journal_test journal_test.cpp)
target_link_libraries(journal_test config log debug2 journal_mock journal util ${CORELIBS})
add_test(journal_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/journal_test ${TEST_SUFFIX})



add_executable(journal_file_test journal_file_test.cpp)
target_link_libraries(journal_file_test config log debug2 journal util ${CORELIBS})
add_test(journal_file_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/journal_file_test ${TEST_SUFFIX})



add_executable(journal_recovery_file_test journal_recovery_file_test.cpp)
target_link_libraries(journal_recovery_file_test config log debug2 journal util ${CORELIBS})
add_test(journal_recovery_file_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/journal_recovery_file_test ${TEST_SUFFIX})
