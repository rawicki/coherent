serialize: serialize.cc Codecs/BigEndianCodec.h Codecs/StandardCodecs.h Encoders/BufferEncoder.h Misc/VirtualClass.h
	g++ -O3 -I. -o serialize serialize.cc

inline: inline_int64_t.cc
	g++ -O3 -I. -c -o inline.o inline_int64_t.cc
	objdump -d inline.o

clean:
	rm -f serialize inline.o $$(find -name "*~")
