#!/bin/sh

# (C) Copyright 2010 Marek Dopiera
# 
# This file is part of CherentDB.
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
# License along with Foobar. If not, see
# http://www.gnu.org/licenses/.

set -e

PROJ_DIR=`dirname $0`/..
cd "${PROJ_DIR}"

if [ `find . -name \*gcda | wc -l` -eq 0 ] ; then
	echo "It seems that you have no coverage data - compile after running \"cmake . -DUSER_COVERAGE=1\" first"
	exit 1;
fi

rm -rf coverage
mkdir coverage
geninfo -o coverage/coverage.out -b . .
genhtml --frames -t "CoherentDB code coverage" -o coverage coverage/coverage.out
