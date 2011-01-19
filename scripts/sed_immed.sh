#/bin/sh

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

if [ $# != 2 ] ; then
	echo "usage: $0 command file"
	exit 1
fi
CMD=$1
FILE=$2
TMP=`mktemp /tmp/sedi.XXXXXX`
if [ $? != 0 ] ; then
	echo "mktemp failed with code $?"
	exit 1
fi
sed -E "$CMD" < "$FILE" >"$TMP"
if [ $? != 0 ] ; then
	echo "sed failed with code $?"
	rm -f "$TMP"
	exit 1
fi
mv "$TMP" "$FILE"
if [ $? != 0 ] ; then
	echo "mv failed with code $?"
	rm -f "$TMP"
	exit 1
fi

