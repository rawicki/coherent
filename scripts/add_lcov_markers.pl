#!/usr/pkg/bin/perl

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


$/='';
while ($_ = <STDIN>) {
	 s/(\b(r_assert|d_assert|LOG)\s*)(\((?:[^\(\)]|(?3))*\))([^\n]*\n)/\/\* LCOV_EXCL_START HACK\*\/\1\3\4\/\* LCOV_EXCL_STOP HACK\*\//g;
	 print
}
