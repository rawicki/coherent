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
#include <codecs/standard_codecs.h>

namespace coherent {
namespace serialization {


template <typename T>
struct big_endian_codec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const T& t)
    {
        t.for_each(enc);
    }
    template <typename Encoder>
    static void encode(Encoder& enc, const T& t, uint32_t v)
    {
        t.for_each(enc, v);
    }

    template <typename Decoder>
    static void decode(Decoder& dec, T& t)
    {
        t.for_each(dec);
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& t, uint32_t v)
    {
        t.for_each(dec, v);
    }
};


template <>
struct big_endian_codec<char>
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
struct big_endian_codec<uint8_t>
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
struct big_endian_int_codec
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


template <> struct big_endian_codec<uint16_t> : public big_endian_int_codec<uint16_t> {};
template <> struct big_endian_codec<uint32_t> : public big_endian_int_codec<uint32_t> {};
template <> struct big_endian_codec<uint64_t> : public big_endian_int_codec<uint64_t> {};

template <> struct big_endian_codec<int8_t> : public big_endian_int_codec<int8_t> {};
template <> struct big_endian_codec<int16_t> : public big_endian_int_codec<int16_t> {};
template <> struct big_endian_codec<int32_t> : public big_endian_int_codec<int32_t> {};
template <> struct big_endian_codec<int64_t> : public big_endian_int_codec<int64_t> {};

template <> struct big_endian_codec<std::string> : public standard_string_codec {};
template <typename T> struct big_endian_codec<std::vector<T> > : public standard_sequence_codec<std::vector<T> > {};
template <typename T> struct big_endian_codec<std::list<T> > : public standard_sequence_codec<std::list<T> > {};
template <typename T> struct big_endian_codec<std::set<T> > : public unique_container_codec<std::set<T> > {};
template <typename K, typename V> struct big_endian_codec<std::map<K,V> > : public unique_container_codec<std::map<K, V> > {};
template <typename F, typename S> struct big_endian_codec<std::pair<F,S> > : public standard_pair_codec<F, S> {};


} // namespace serialization
} // namespace coherent

#endif /* BIG_ENDIAN_CODEC_H */
