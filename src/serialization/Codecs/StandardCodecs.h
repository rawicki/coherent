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

#ifndef STANDARD_CODECS_H
#define STANDARD_CODECS_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <inttypes.h>
#include "Misc/VirtualClass.h"


template <typename T>
struct StandardSequenceCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const T& s)
    {
        enc(static_cast<uint32_t>(s.size()));
        for (typename T::const_iterator it=s.begin(); it!=s.end(); ++it)
        {
            enc(*it);
        }
    }
    template <typename Decoder>
    static void decode(Decoder& dec, T& s)
    {
        uint32_t x;
        dec(x);
        s.resize(x);
        for (typename T::iterator it=s.begin(); it!=s.end(); ++it)
        {
            dec(*it);
        }
    }
};

struct StandardStringCodec : public StandardSequenceCodec<std::string>
{
};

template <typename Container>
struct UniqueContainerCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const Container& cont)
    {
        enc(static_cast<uint32_t>(cont.size()));
        for (typename Container::const_iterator it=cont.begin(); it!=cont.end(); ++it)
        {
            enc(*it);
        }
    }

    template <typename Decoder>
    static void decode(Decoder& dec, Container& cont)
    {
        uint32_t x;
        dec(x);
        cont.clear();
        for (uint32_t i=0; i<x; i++) {
            typename Container::value_type val;
            dec(val);
            cont.insert(val);
        }
    }
};

template <typename F, typename S>
struct StandardPairCodec
{
    template <typename Encoder>
    static void encode(Encoder& enc, const std::pair<F,S>& p)
    {
        enc(p.first);
        enc(p.second);
    }

    template <typename Decoder>
    static void decode(Decoder& dec, std::pair<F,S>& p)
    {
        dec(p.first);
        dec(p.second);
    }
};

template <typename T, typename VL>
struct StandardVirtuaClassCodec
{
    typedef Virtual<T, VL> Type;

    template <typename Encoder>
    static void encode(Encoder& enc, const Type& t)
    {
        t.encode(enc);
    }

    template <typename Decoder>
    static void decode(Decoder& dec, Type& t)
    {
        t.decode(dec);
    }
};


#endif /* STANDARD_CODECS_H */
