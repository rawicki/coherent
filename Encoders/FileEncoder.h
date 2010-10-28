#ifndef FILE_ENCODER_H
#define FILE_ENCODER_H

#include <vector>
#include <iterator>
#include <assert.h>
#include <unistd.h>


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

private:
    int fd_;
    size_t bufferSize_;
    std::vector<char> buffer_;
    size_t pos_;
};


#endif /* FILE_ENCODER_H */
