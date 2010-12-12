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

#ifndef LITTLE_ENDIAN_CODEC_H
#define LITTLE_ENDIAN_CODEC_H

#include <inttypes.h>
#include "Codecs/StandardCodecs.h"


template <typename T>
struct LittleEndianCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const T& t)
    {
        t.forEach(enc);
    }
    template <typename Encoder>
    static void encode(Encoder& enc, const T& t, uint32_t v)
    {
        t.forEach(enc, v);
    }

    template <typename Decoder>
    static void decode(Decoder& dec, T& t)
    {
        t.forEach(dec);
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& t, uint32_t v)
    {
        t.forEach(dec, v);
    }
};


template <>
struct LittleEndianCodec<char>
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
struct LittleEndianCodec<uint8_t>
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
struct LittleEndianIntCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const T& x)
    {
        size_t i = 0;
        while (i < 8*sizeof(T)) {
            enc.write_char( static_cast<uint8_t>( (x>>i) & 0xff) );
            i += 8;
        }
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& x)
    {
        size_t i = 0;
        x = 0;
        while (i<sizeof(T)) {
            uint8_t c = dec.get_char();
            x |= (((T)c)<<(8*i));
            i++;
        }
    }
};


template <> struct LittleEndianCodec<uint16_t> : public LittleEndianIntCodec<uint16_t> {};
template <> struct LittleEndianCodec<uint32_t> : public LittleEndianIntCodec<uint32_t> {};
template <> struct LittleEndianCodec<uint64_t> : public LittleEndianIntCodec<uint64_t> {};

template <> struct LittleEndianCodec<int8_t> : public LittleEndianIntCodec<int8_t> {};
template <> struct LittleEndianCodec<int16_t> : public LittleEndianIntCodec<int16_t> {};
template <> struct LittleEndianCodec<int32_t> : public LittleEndianIntCodec<int32_t> {};
template <> struct LittleEndianCodec<int64_t> : public LittleEndianIntCodec<int64_t> {};

template <> struct LittleEndianCodec<std::string> : public StandardStringCodec {};
template <typename T> struct LittleEndianCodec<std::vector<T> > : public StandardSequenceCodec<std::vector<T> > {};
template <typename T> struct LittleEndianCodec<std::list<T> > : public StandardSequenceCodec<std::list<T> > {};
template <typename T> struct LittleEndianCodec<std::set<T> > : public UniqueContainerCodec<std::set<T> > {};
template <typename K, typename V> struct LittleEndianCodec<std::map<K,V> > : public UniqueContainerCodec<std::map<K, V> > {};
template <typename F, typename S> struct LittleEndianCodec<std::pair<F,S> > : public StandardPairCodec<F, S> {};

template <typename T, typename VL, typename Ptr> struct LittleEndianCodec<Virtual<T, VL, Ptr> > : public StandardVirtuaClassCodec<T, VL, Ptr> {};


#endif /* LITTLE_ENDIAN_CODEC_H */
