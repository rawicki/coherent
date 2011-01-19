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

#include "codecs/big_endian.h"
#include "codecs/little_endian.h"

template <typename T> struct codec : public little_endian_codec<T> {};

typedef int64_t type;

struct encoder
{
    void write_char(char);
};

struct decoder
{
    char get_char();
};

void funkcja_enc(const type& x)
{
    encoder enc;
    codec<type>::encode(enc, x);
}

void funkcja_dec(type& x)
{
    decoder dec;
    codec<type>::decode(dec, x);
}

