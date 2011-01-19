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

#ifndef BUFFER_ENCODER_H
#define BUFFER_ENCODER_H

#include <stdexcept>
#include <vector>
#include <iterator>
#include <string.h>
#include <inttypes.h>

namespace coherent {
namespace serialization {


template <template <typename T> class Codec>
struct buffer_encoder
{
    buffer_encoder(std::vector<char>& buffer)
        : buffer_(buffer)
    {
    }
    template <typename T>
    buffer_encoder& operator() (const T& x)
    {
        Codec<T>::encode(*this, x);
        return *this;
    }
    template <typename T>
    buffer_encoder& operator() (const T& x, uint32_t v)
    {
        Codec<T>::encode(*this, x, v);
        return *this;
    }
    void write_char(char c)
    {
        buffer_.push_back(c);
    }
    //optimizer
    void memcpy(const char * src, size_t n)
    {
        size_t size_ = buffer_.size();
        buffer_.resize(size_+n);
        ::memcpy( (void*)(&(buffer_[size_])), (void*)src, n);
    }
private:
    std::vector<char>& buffer_;
};

template <template <typename T> class Codec>
struct buffer_decoder
{
    buffer_decoder(std::vector<char>::const_iterator& begin, std::vector<char>::const_iterator end) : begin_(begin), end_(end)
    {
    }
    template <typename T>
    buffer_decoder& operator() (T & x)
    {
        Codec<T>::decode(*this, x);
        return *this;
    }
    template <typename T>
    buffer_decoder& operator() (T & x, uint32_t v)
    {
        Codec<T>::decode(*this, x, v);
        return *this;
    }
    char get_char()
    {
        if (begin_==end_) {
            throw std::out_of_range("buffer_decoder::get_char");
        }
        return *(begin_++);
    }
    //optimizer
    void memcpy(char * dest, size_t n)
    {
        if (end_-begin_ < n) {
            throw std::out_of_range("buffer_decoder::memcpy");
        }
        ::memcpy( (void*)dest, (void*)(&(*begin_)), n);
        begin_ += n;
    }
private:
    std::vector<char>::const_iterator& begin_;
    std::vector<char>::const_iterator end_;
};


} // namespace serialization
} // namespace coherent

#endif /* BUFFER_ENCODER_H */
