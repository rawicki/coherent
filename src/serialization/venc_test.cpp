/*
 * (C) Copyright 2010 Tomasz Zolnowski
 * 
 * This file is part of CoherentDB.
 * 
 * CoherentDB is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CoherentDB is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with CoherentDB. If not, see
 * http://www.gnu.org/licenses/.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <inttypes.h>

#define DUMP_TYPEINFO 1

#include "encoders/virtual_encoder.h"
#include "encoders/virtual_decoder.h"
#include "encoders/file_encoder.h"
#include "encoders/buffer_encoder.h"

#include "codecs/little_endian.h"


typedef make_list3<std::string, uint32_t, int64_t>::value       List1;
typedef CreateEncoderSet<List1>         EncoderSet;
typedef EncoderSet::EncoderAbs          EncoderAbs;
typedef EncoderSet::EncoderType         EncoderType;

typedef EncoderSet::EncoderImpl< buffer_encoder<little_endian_codec> >   buffer_encoderImpl;
typedef EncoderSet::EncoderImpl< file_encoder<little_endian_codec> >     file_encoderImpl;

typedef CreateDecoderSet<List1>         DecoderSet;
typedef DecoderSet::DecoderAbs          DecoderAbs;
typedef DecoderSet::DecoderType         DecoderType;

typedef DecoderSet::DecoderImpl< buffer_decoder<little_endian_codec> >   buffer_decoderImpl;
typedef DecoderSet::DecoderImpl< file_decoder<little_endian_codec> >     file_decoderImpl;


void foo(EncoderType& enc)
{
    std::string s("Hello world!");
    uint32_t x = 7;
    int64_t z = 123456789LL;
    enc(x);
    enc(s);
    enc(z);

    //std::vector<uint8_t> v;
    //enc(v);
}

void bar(DecoderType& dec)
{
    std::string s;
    uint32_t x;
    int64_t z;

    dec(x);
    dec(s);
    dec(z);

    std::cout << "bar: [" << s << "], " << x << ", " << z << std::endl;
}

int main(int argc, char **argv)
{
    std::cout << "Hello world!" << std::endl;

    if (argc<2) {
        std::cerr << "params?" << std::endl;
        return 1;
    }


    //encoding
    EncoderType enc;
    std::string cmd(argv[1]);

    std::vector<char> buff1;
    std::vector<char> buff2;
    std::vector<char> buff3;

    buffer_encoder<little_endian_codec> be1(buff1);
    buffer_encoder<little_endian_codec> be2(buff2);
    buffer_encoder<little_endian_codec> be3(buff3);

    if (cmd == "1") enc = EncoderType(new buffer_encoderImpl(be1));
    if (cmd == "2") enc = EncoderType(new buffer_encoderImpl(be2));
    if (cmd == "3") enc = EncoderType(new buffer_encoderImpl(be3));

    foo(enc);
    std::cout << buff1.size() << " " << buff2.size() << " " << buff3.size() << std::endl;

    //decoding
    DecoderType dec;

    std::vector<char>::const_iterator bi1 = buff1.begin();
    std::vector<char>::const_iterator bi2 = buff2.begin();
    std::vector<char>::const_iterator bi3 = buff3.begin();

    buffer_decoder<little_endian_codec> bd1(bi1, buff1.end());
    buffer_decoder<little_endian_codec> bd2(bi2, buff2.end());
    buffer_decoder<little_endian_codec> bd3(bi3, buff3.end());

    if (buff1.size()) dec = DecoderType(new buffer_decoderImpl(bd1));
    if (buff2.size()) dec = DecoderType(new buffer_decoderImpl(bd2));
    if (buff3.size()) dec = DecoderType(new buffer_decoderImpl(bd3));

    bar(dec);

    return 0;
}

