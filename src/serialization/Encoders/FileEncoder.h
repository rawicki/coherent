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

#ifndef FILE_ENCODER_H
#define FILE_ENCODER_H

#include <vector>
#include <iterator>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>


template <template <typename T> class Codec>
struct FileEncoder
{
    FileEncoder(int fd, size_t bufferSize = 4096) : fd_(fd), bufferSize_(bufferSize)
    {
    }

    template <typename T>
    FileEncoder& operator() (const T& x)
    {
        Codec<T>::encode(*this, x);
        return *this;
    }
    template <typename T>
    FileEncoder& operator() (const T& x, uint32_t v)
    {
        Codec<T>::encode(*this, x, v);
        return *this;
    }

    void write_char(char c)
    {
        if (buffer_.size() >= bufferSize_)
            flush();
        buffer_.push_back(c);
    }

    void flush()
    {
        if (buffer_.size()) {
            ssize_t len = write(fd_, &(*buffer_.begin()), buffer_.size());
            if (len!=buffer_.size()) {
                throw "write error";
            }
            buffer_.clear();
        }
    }
    ~FileEncoder()
    {
        flush();
    }
    //optimizer
    void memcpy(const char * src, size_t n)
    {
        for (;;) {
            if (buffer_.size() >= bufferSize_) {
                flush();
            }
            size_t toWrite = std::min<size_t>(n, bufferSize_-buffer_.size());
            size_t size_ = buffer_.size();
            buffer_.resize(size_+toWrite);

            ::memcpy( (void*)(&(buffer_[size_])), (void*)src, toWrite);

            src += toWrite;
            n -= toWrite;

            if (n==0)
                break;
        }
    }
private:
    int fd_;
    size_t bufferSize_;
    std::vector<char> buffer_;
};


template <template <typename T> class Codec>
struct FileDecoder
{
    FileDecoder(int fd, size_t bufferSize = 4096) : fd_(fd), bufferSize_(bufferSize)
    {
    }

    template <typename T>
    FileDecoder& operator() (T & x)
    {
        Codec<T>::decode(*this, x);
        return *this;
    }
    template <typename T>
    FileDecoder& operator() (T & x, uint32_t v)
    {
        Codec<T>::decode(*this, x, v);
        return *this;
    }
    char get_char()
    {
        if (pos_>=buffer_.size()) {
            advance();
        }
        return buffer_[pos_++];
    }
    void advance()
    {
        buffer_.resize(bufferSize_);
        ssize_t len = read(fd_, &(*buffer_.begin()), buffer_.size());
        if (len<=0) {
            throw "read error";
        }
        buffer_.resize(len);
        pos_ = 0;
    }

    //optimizer
    void memcpy(char * dest, size_t n)
    {
        for (;;) {
            if (pos_>=buffer_.size()) {
                advance();
            }
            size_t toRead = std::min<size_t>(n, buffer_.size()-pos_);
            ::memcpy( (void*)dest, (void*)(&(buffer_[pos_])), toRead);

            pos_ += toRead;
            n -= toRead;
            if (n==0)
                break;
            dest += toRead;
        }
    }
private:
    int fd_;
    size_t bufferSize_;
    std::vector<char> buffer_;
    size_t pos_;
};


#endif /* FILE_ENCODER_H */
