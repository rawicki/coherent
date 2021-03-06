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


PATHS		:=	encoders ../include/misc ../include/serialization/codecs ../include/serialization/encoders
HEADERS		:=	$(foreach dir, $(PATHS), $(wildcard $(dir)/*.h))
INCDIRS		:= -I. -I../include -I../include/misc -I../include/serialization
LIBS		:= -L../../bin/ -llog -ldebug2 -llog4cxx

all: serialize gen_test venc_test version_test vc_example list_example

headers:
	@echo $(HEADERS)

.PHONY: headers

serialize: serialize.cpp $(HEADERS)
	g++ -O3 $(INCDIRS) -o serialize serialize.cpp $(LIBS)

inline: inline_int64_t.cpp $(HEADERS)
	g++ -O3 $(INCDIRS) -c -o inline.o inline_int64_t.cpp
	objdump -d inline.o

gen_test: vc_gen_test.cpp gen_main.cpp $(HEADERS)
	g++ -O3 -o vc_gen_test vc_gen_test.cpp
	./vc_gen_test 6 > gen_test.h
	./vc_gen_test 6 static > gen_test.cpp
	g++ -O3 $(INCDIRS) -c -o gen_test.o gen_test.cpp
	g++ -O3 $(INCDIRS) -c -o gen_main.o gen_main.cpp
	g++ -o gen_test gen_test.o gen_main.o $(LIBS)

venc_test: venc_test.cpp $(HEADERS)
	g++ -O3 $(INCDIRS) -c -o venc_test.o venc_test.cpp
	g++ -o venc_test venc_test.o $(LIBS)

version_test: version_test.cpp $(HEADERS)
	g++ -O3 $(INCDIRS) -c -o version_test.o version_test.cpp
	g++ -o version_test version_test.o $(LIBS)

vc_example: vc_example.cpp $(HEADERS)
	g++ -O3 $(INCDIRS) -c -o vc_example.o vc_example.cpp
	g++ -o vc_example vc_example.o $(LIBS)

list_example: list_example.cpp ../include/misc/type_list.h
	g++ -O3 $(INCDIRS) -c -o list_example.o list_example.cpp
	g++ -o list_example list_example.o $(LIBS)

clean:
	rm -f serialize *.o *test vc_example list_example  gen_test.h gen_test.cpp $$(find -name "*~")


