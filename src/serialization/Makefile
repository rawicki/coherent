# (C) Copyright 2010 Tomasz Zolnowski
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


PATHS		:=	Codecs Encoders Misc
HEADERS		:=	$(foreach dir, $(PATHS), $(wildcard $(dir)/*.h))


all: serialize gen_test venc_test version_test vc_example list_example


serialize: serialize.cc $(HEADERS)
	g++ -O3 -I. -o serialize serialize.cc

inline: inline_int64_t.cc $(HEADERS)
	g++ -O3 -I. -c -o inline.o inline_int64_t.cc
	objdump -d inline.o

gen_test: vc_gen_test.cc gen_main.cc $(HEADERS)
	g++ -O3 -o vc_gen_test vc_gen_test.cc
	./vc_gen_test 6 > gen_test.h
	./vc_gen_test 6 static > gen_test.cc
	g++ -O3 -I. -c -o gen_test.o gen_test.cc
	g++ -O3 -I. -c -o gen_main.o gen_main.cc
	g++ -o gen_test gen_test.o gen_main.o

venc_test: venc_test.cc $(HEADERS)
	g++ -O3 -I. -c -o venc_test.o venc_test.cc
	g++ -o venc_test venc_test.o

version_test: version_test.cc $(HEADERS)
	g++ -O3 -I. -c -o version_test.o version_test.cc
	g++ -o version_test version_test.o

vc_example: vc_example.cc $(HEADERS)
	g++ -O3 -I. -c -o vc_example.o vc_example.cc
	g++ -o vc_example vc_example.o

list_example: list_example.cc Misc/TypeList.h
	g++ -O3 -I. -c -o list_example.o list_example.cc
	g++ -o list_example list_example.o

clean:
	rm -f serialize *.o *test vc_example list_example  gen_test.h gen_test.cc $$(find -name "*~")


