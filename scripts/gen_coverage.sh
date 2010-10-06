#!/bin/bash
#!/usr/pkg/bin/bash

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

set -e

function usage
{
	echo "usage: $0 source_dir binary_dir"
	echo "If you have run \"cmake .\" in source directory, then source_dir"
	echo "and binary_dir are the same"
	exit 1
}

OLD_PWD="`pwd`"
if [ $# -ne 2 ] ; then
	usage
fi
SOURCE_DIR=$1
BINARY_DIR=$2

COVERAGE_DIR="${OLD_PWD}/coverage"
COVERAGE_OUT="${COVERAGE_DIR}/coverage.out"
COVERAGE_OUT_STRIPPED="${COVERAGE_DIR}/coverage_stripped.out"

rm -rf "${COVERAGE_DIR}"
mkdir -m755 "${COVERAGE_DIR}"

cd ${BINARY_DIR}
if [ `find . -name \*gcda | wc -l` -eq 0 ] ; then
	echo "It seems that you have no coverage data - compile after running \"cmake . -DUSER_COVERAGE=1\" first"
	exit 1;
fi
geninfo -o "${COVERAGE_OUT}" -b . .
cd ${SOURCE_DIR}
lcov -r ${COVERAGE_OUT} /usr/\* > ${COVERAGE_OUT_STRIPPED}
genhtml -t "CoherentDB code coverage" -o ${COVERAGE_DIR} ${COVERAGE_OUT_STRIPPED}
