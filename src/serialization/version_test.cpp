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
#include <inttypes.h>
#include <vector>

#include "codecs/little_endian.h"
#include "encoders/buffer_encoder.h"
#include "encoders/file_encoder.h"

#define DUMP_TYPEINFO 1

#include "encoders/encoders_pimpl.h"

using namespace coherent::serialization;
using namespace coherent::misc;


struct A_old
{
    A_old() {}
    A_old(const std::string& ax, uint32_t ay) : ax_(ax), ay_(ay) {}

    template <typename F>
    void for_each(F & f)
    {
        f(ax_);
        f(ay_);
    }
    template <typename F>
    void for_each(F & f) const
    {
        f(ax_);
        f(ay_);
    }
    std::string ax_;
    uint32_t ay_;

    friend std::ostream& operator<< (std::ostream& os, const A_old& a0)
    {
        return os << "A0(" << a0.ax_ << "," << a0.ay_ << ")";
    }
};

struct A
{
    A() {}
    A(const std::string& ax, uint32_t ay, uint64_t az) : ax_(ax), ay_(ay), az_(az) {}

    template <typename F>
    void for_each(F & f, uint32_t v = 1)
    {
        f(ax_);
        f(ay_);
        if (v) {
            f(az_);
        }
        else {
            //set default value
            az_ = 7;
        }
    }
    template <typename F>
    void for_each(F & f) const   //example: no versions for encoding
    {
        f(ax_);
        f(ay_);
        f(az_);
    }
    template <typename F>
    void for_each(F & f, uint32_t v) const
    {
        f(ax_);
        f(ay_);
        if (v) {
            f(az_);
        }
    }

    friend std::ostream& operator<< (std::ostream& os, const A& a)
    {
        return os << "A(" << a.ax_ << "," << a.ay_ << "," << a.az_ << ")";
    }

    std::string ax_;
    uint32_t ay_;
    uint64_t az_;
};



//vdec + versioning
typedef make_list5<std::string, uint32_t, int64_t, A_old, detail::__version<A> >::value  List1;
typedef create_decoder_set<List1>       DecoderSet;
typedef DecoderSet::decoder_abs         DecoderAbs;
typedef DecoderSet::decoder_type        DecoderType;

typedef DecoderSet::decoder_impl< buffer_decoder<little_endian_codec> >::type   buffer_decoder_impl;
typedef DecoderSet::decoder_impl< file_decoder<little_endian_codec> >::type     file_decoder_impl;

//venc + versioning
typedef create_encoder_set<List1>       EncoderSet;
typedef EncoderSet::encoder_abs         EncoderAbs;
typedef EncoderSet::encoder_type        EncoderType;

typedef EncoderSet::encoder_impl< buffer_encoder<little_endian_codec> >::type   buffer_encoder_impl;


typedef EncoderSet::create_encoder_impl<
                detail::field_policy<detail::encoder_qualifier, buffer_encoder<little_endian_codec> >
            >::type buffer_encoder_impl2;

typedef EncoderSet::create_encoder_impl<
                detail::field_policy<detail::encoder_qualifier, file_encoder<little_endian_codec> >
            >::type file_encoder_impl;


// class B definitions
struct B
{
    B() {}
    B(uint64_t f1, uint64_t f2) : v_(2), f1_(f1), f2_(f2)
    {
    }
    template <typename F>
    void for_each(F & f) const   //encode only default
    {
        f(f1_);
        f(f2_);
    }
    template <typename F>
    void for_each(F & f, uint32_t v = 2) //decode with or without version
    {
        if (v>0) f(f1_); else f1_ = -1;
        if (v>1) f(f2_); else f2_ = -1;
        v_ = v;
    }

    friend std::ostream& operator<< (std::ostream& os, const B& b)
    {
        return os << "B(" << b.v_ << "," << b.f1_ << "," << b.f2_ << ")";
    }

    uint32_t v_;
    int64_t f1_;
    int64_t f2_;
};

typedef make_list3<std::string, B, int64_t>::value ListB1; //enc
typedef make_list2<std::string, detail::__both<B> >::value ListB2; //dec

typedef create_encoder_set<ListB1>::encoder_type Encoder_B;
typedef create_decoder_set<ListB2>::decoder_type Decoder_B;

typedef create_encoder_set<ListB1>::create_encoder_impl< detail::field_policy<detail::encoder_qualifier, buffer_encoder<little_endian_codec> > >::type buffer_encoder_B;
typedef create_decoder_set<ListB2>::create_decoder_impl< detail::field_policy<detail::decoder_qualifier, buffer_decoder<little_endian_codec> > >::type buffer_decoder_B;


void foo(DecoderType& dec)
{
    std::cout << "BEGIN A decoding" << std::endl;

    std::string s;
    uint32_t x;
    int64_t z;

    A_old a0;
    A a1;


    dec(a0);
    dec(a1, 1);

    dec(x);
    dec(z);
    dec(s);

    std::cout << "A decoding results" << std::endl;
    std::cout << "a0(" << a0.ax_ << "," << a0.ay_ << /*"," << a0.az_ <<*/ ")" << std::endl;
    std::cout << "a1(" << a1.ax_ << "," << a1.ay_ << "," << a1.az_ << ")" << std::endl;

    std::cout << "x: " << x << ", z: " << z << ", s: " << s << std::endl;

    std::cout << "END A decoding" << std::endl;
}


void bar(EncoderType& enc)
{
    A_old ao;
    ao.ax_ = "enc1";
    ao.ay_ = 1;

    A ap;
    ap.ax_ = "enc_ap";
    ap.ay_ = 17;
    ap.az_ = 19;    //should not be encoded

    A az;
    az.ax_ = "enc_az";
    az.ay_ = 21;
    az.az_ = 23;

    enc(ao)(ap, 0)(az, 1);
}


struct PrintBuf
{
    const std::vector<char>& buf_;

    PrintBuf(const std::vector<char>& buf) : buf_(buf)
    {
    }
    friend std::ostream& operator<< (std::ostream& os, const PrintBuf& pb)
    {
        const std::vector<char>& buf = pb.buf_;
        for (std::vector<char>::const_iterator it=buf.begin(); it!=buf.end(); ++it)
        {
            uint8_t c = *it;
            if (32<=c && c<127)
                os << (char)c;
            else
                os << '\\'
                    << (char)((c/16>9) ? (c/16-10+'a') : (c/16+'0'))
                    << (char)((c%16>9) ? (c%16-10+'a') : (c%16+'0'));
        }
        return os;
    }
};


int main()
{
    std::vector<char> buf;

    {
        //simple encode some objects
        buffer_encoder<little_endian_codec> enc(buf);

        A_old ao("xyz", (13<<8)+7);
        A an("obiekt A", 78, (1LL<<40) + (15LL<<32) + 65);

        enc(ao)(an)((uint32_t)777)((int64_t)13051)(std::string("hello!"));
    }

    std::cout << PrintBuf(buf) << std::endl;

    {
        //simple decode using versioning
        std::vector<char>::const_iterator it = buf.begin();
        buffer_decoder<little_endian_codec> dec(it, buf.end());

        A a1, a2;
        dec(a1, 0)(a2); //a1 with old version

        std::cout << "a1(" << a1.ax_ << "," << a1.ay_ << "," << a1.az_ << ")" << std::endl;
        std::cout << "a2(" << a2.ax_ << "," << a2.ay_ << "," << a2.az_ << ")" << std::endl;
    }

    {
        //use virtual encoder with versioning

        std::cout << "BEGIN A encoding" << std::endl;

        A_old ao("stary format", 111);
        A a1("po staremu", 112, 101);
        A a2("po nowemu", 113, 251);

        //std::cout << buf.size() << " " << (void*)(&buf[0]) << std::endl;
        EncoderType enc = EncoderType(new buffer_encoder_impl2(buf));
        enc(ao)(a1, 0)(a2, 5);

        std::cout << "END A encoding" << std::endl;
    }
    std::cout << PrintBuf(buf) << std::endl;

    {
        //use virtual decoder

        std::vector<char>::const_iterator it = buf.begin();
        buffer_decoder<little_endian_codec> bd(it, buf.end());
        DecoderType dec = DecoderType(new buffer_decoder_impl(bd));

        foo(dec);
    }

    buf.clear();
    {
        std::cout << "BEGIN B encoding" << std::endl;

        Encoder_B enc(new buffer_encoder_B(buf));
        enc(std::string("Hello!"))(B(4,7));  //hello / B-2
        enc((int64_t)10);   //B-1
        //nop for B-0
        enc((int64_t)13)((int64_t)16);
        enc(std::string("bye"));

        std::cout << "END B encoding" << std::endl;
    }
    {
        std::cout << "BEGIN B decoding" << std::endl;

        std::vector<char>::const_iterator it = buf.begin();
        std::vector<char>::const_iterator end = buf.end();
        Decoder_B dec(new buffer_decoder_B(it, end));

        std::string hello, bye;
        B b1, b2, b3, b4;
        dec(hello)(b1,5)(b2,1)(b3,0)(b4)(bye);

        std::cout << "Decoded: " << hello << ", " << b1 << ", " << b2 << ", " << b3 << ", " << b4 << ", " << bye << std::endl;

        std::cout << "END B decoding" << std::endl;
    }

    return 0;
}

