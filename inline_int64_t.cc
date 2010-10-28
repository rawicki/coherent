#include "Codecs/BigEndianCodec.h"

struct Enc
{
    void write_char(char);
};

void funkcja(int64_t x)
{
    Enc enc;
    BigEndianCodec<int64_t>::encode(enc, x);
}

