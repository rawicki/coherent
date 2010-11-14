serialize: serialize.cc Codecs/BigEndianCodec.h Codecs/LittleEndianCodec.h Codecs/StandardCodecs.h Encoders/BufferEncoder.h Misc/VirtualClass.h
	g++ -O3 -I. -o serialize serialize.cc

inline: inline_int64_t.cc
	g++ -O3 -I. -c -o inline.o inline_int64_t.cc
	objdump -d inline.o

gen_test: vc_gen_test.cc gen_main.cc
	g++ -O3 -o vc_gen_test vc_gen_test.cc
	./vc_gen_test 6 > gen_test.h
	./vc_gen_test 6 static > gen_test.cc
	g++ -O3 -I. -c -o gen_test.o gen_test.cc
	g++ -O3 -I. -c -o gen_main.o gen_main.cc
	g++ -o gen_test gen_test.o gen_main.o

clean:
	rm -f serialize *.o vc_gen_test gen_test gen_test.h gen_test.cc $$(find -name "*~")
