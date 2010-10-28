#include "Codecs/BigEndianCodec.h"
#include "Codecs/LittleEndianCodec.h"

template <typename T> struct Codec : public LittleEndianCodec<T> {};
typedef int64_t Type;

struct Enc
{
    void write_char(char);
};

struct Dec
{
    char get_char();
};

void funkcja_enc(Type x)
{
    Enc enc;
    Codec<Type>::encode(enc, x);
}

void funkcja_dec(Type& x)
{
    Dec dec;
    Codec<Type>::decode(dec, x);
}

