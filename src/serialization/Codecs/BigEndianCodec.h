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

#ifndef BIG_ENDIAN_CODEC_H
#define BIG_ENDIAN_CODEC_H

#include <inttypes.h>
#include "Codecs/StandardCodecs.h"


template <typename T>
struct BigEndianCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const T& t)
    {
        t.forEach(enc);
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& t)
    {
        t.forEach(dec);
    }
};


template <>
struct BigEndianCodec<char>
{
    template <typename Encoder>
    static void encode(Encoder& enc, char t)
    {
        enc.write_char(t);
    }
    template <typename Decoder>
    static void decode(Decoder& dec, char& t)
    {
        t = dec.get_char();
    }
};

template <>
struct BigEndianCodec<uint8_t>
{
    template <typename Encoder>
    static void encode(Encoder& enc, uint8_t u)
    {
        enc.write_char(static_cast<char>(u));
    }
    template <typename Decoder>
    static void decode(Decoder& dec, uint8_t& u)
    {
        u = dec.get_char();
    };
};

template <typename T>
struct BigEndianIntCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const T& x)
    {
        size_t i = 8 * sizeof(T);
        while (i) {
            i -= 8;
            enc.write_char( static_cast<uint8_t>(x>>i) );
        }
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& x)
    {
        size_t i = 0;
        x = 0;
        while (i++<sizeof(T)) {
            uint8_t c = dec.get_char();
            x = (x<<8) | c;
        }
    }
};


template <> struct BigEndianCodec<uint16_t> : public BigEndianIntCodec<uint16_t> {};
template <> struct BigEndianCodec<uint32_t> : public BigEndianIntCodec<uint32_t> {};
template <> struct BigEndianCodec<uint64_t> : public BigEndianIntCodec<uint64_t> {};

template <> struct BigEndianCodec<int8_t> : public BigEndianIntCodec<int8_t> {};
template <> struct BigEndianCodec<int16_t> : public BigEndianIntCodec<int16_t> {};
template <> struct BigEndianCodec<int32_t> : public BigEndianIntCodec<int32_t> {};
template <> struct BigEndianCodec<int64_t> : public BigEndianIntCodec<int64_t> {};

template <> struct BigEndianCodec<std::string> : public StandardStringCodec {};
template <typename T> struct BigEndianCodec<std::vector<T> > : public StandardSequenceCodec<std::vector<T> > {};
template <typename T> struct BigEndianCodec<std::list<T> > : public StandardSequenceCodec<std::list<T> > {};
template <typename T> struct BigEndianCodec<std::set<T> > : public UniqueContainerCodec<std::set<T> > {};
template <typename K, typename V> struct BigEndianCodec<std::map<K,V> > : public UniqueContainerCodec<std::map<K, V> > {};
template <typename F, typename S> struct BigEndianCodec<std::pair<F,S> > : public StandardPairCodec<F, S> {};

template <typename T, typename VL> struct BigEndianCodec<Virtual<T, VL> > : public StandardVirtuaClassCodec<T, VL> {};


#endif /* BIG_ENDIAN_CODEC_H */
