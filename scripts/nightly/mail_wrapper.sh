#!/bin/bash
#!/usr/pkg/bin/bash

# (C) Copyright 2010 XXXXXX
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


cd "`dirname $0`"

MESSAGE_HEADER="message_header"
REPORT_LOG="report_log"

if [ ! -f "${MESSAGE_HEADER}" ] ; then
	echo "There is no message header, please provide one"
	exit 1;
fi

./tester.sh > "${REPORT_LOG}"
RESULT=$?

if [ "${RESULT}" -eq 2 ] ; then
	echo "Nothing new to check"
	exit 0
fi

if [ "${RESULT}" -eq 0 ] ; then
	SUBJECT="UT report: SUCCESS"
else
	SUBJECT="UT report: FAILURE"
fi

( cat ${MESSAGE_HEADER} ; echo "Subject: ${SUBJECT}" ; echo ; cat "${REPORT_LOG}" ) | sendmail -t



