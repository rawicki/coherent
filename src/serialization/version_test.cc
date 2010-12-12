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

#include "Codecs/LittleEndianCodec.h"
#include "Encoders/BufferEncoder.h"


struct A_old
{
    template <typename F>
    void forEach(F & f)
    {
        f(ax_);
        f(ay_);
    }
    template <typename F>
    void forEach(F & f) const
    {
        f(ax_);
        f(ay_);
    }
    std::string ax_;
    uint32_t ay_;
};

struct A
{
    template <typename F>
    void forEach(F & f, uint32_t v = 1)
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
    void forEach(F & f) const   //example: no versions for encoding
    {
        f(ax_);
        f(ay_);
        f(az_);
    }

    std::string ax_;
    uint32_t ay_;
    uint64_t az_;
};


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
        BufferEncoder<LittleEndianCodec> enc(buf);

        A_old ao;
        ao.ax_ = "testowy napis";
        ao.ay_ = (13<<8)+7;

        enc(ao);

        A an;
        an.ax_ = "obiekt klasy A";
        an.ay_ = 78;
        an.az_ = (1LL<<40) + (15LL<<32) + 65;

        enc(an);
    }

    std::cout << PrintBuf(buf) << std::endl;

    {
        std::vector<char>::const_iterator it = buf.begin();
        BufferDecoder<LittleEndianCodec> dec(it, buf.end());

        A a1;
        dec(a1, 0);     //choose old version

        A a2;
        dec(a2);

        std::cout << "a1(" << a1.ax_ << "," << a1.ay_ << "," << a1.az_ << ")" << std::endl;
        std::cout << "a2(" << a2.ax_ << "," << a2.ay_ << "," << a2.az_ << ")" << std::endl;
    }

    return 0;
}

