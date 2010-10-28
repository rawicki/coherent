#ifndef BUFFER_ENCODER_H
#define BUFFER_ENCODER_H

#include <vector>
#include <iterator>
#include <assert.h>


template <template <typename T> class Codec>
struct BufferEncoder
{
    BufferEncoder(std::vector<char>& buffer) : it_(std::back_inserter(buffer))
    {
    }
    template <typename T>
    BufferEncoder& operator() (const T& x)
    {
        Codec<T>::encode(*this, x);
        return *this;
    }
    void write_char(char c)
    {
        *it_++ = c;
    }
private:
    std::back_insert_iterator<std::vector<char> > it_;
};

template <template <typename T> class Codec>
struct BufferDecoder
{
    BufferDecoder(std::vector<char>::const_iterator& begin, std::vector<char>::const_iterator end) : begin_(begin), end_(end)
    {
    }
    template <typename T>
    BufferDecoder& operator() (T & x)
    {
        Codec<T>::decode(*this, x);
        return *this;
    }
    char get_char()
    {
        assert(begin_!=end_);
        return *(begin_++);
    }
private:
    std::vector<char>::const_iterator& begin_;
    std::vector<char>::const_iterator end_;
};


#endif /* BUFFER_ENCODER_H */
