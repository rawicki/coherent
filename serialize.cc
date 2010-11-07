#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <typeinfo>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <boost/random.hpp>
#include "Codecs/BigEndianCodec.h"
#include "Codecs/LittleEndianCodec.h"
#include "Codecs/Optimizers.h"
#include "Encoders/BufferEncoder.h"
#include "Encoders/FileEncoder.h"
#include "Misc/VectorOutput.h"


//virtual structurs
struct XA
{
    typedef uint8_t TagType;
    virtual TagType getTag() const = 0;

    virtual void print(std::ostream& os) const { os << "XA(" << xa << ")"; }
    uint32_t xa;
    template <typename F> void forEach(F & f)
    {
        f(xa);
    }
    template <typename F> void forEach(F & f) const
    {
        f(xa);
    }

    friend std::ostream& operator<< (std::ostream& os, const XA& x)
    {
        x.print(os);
        return os;
    }
};

#define DEFTAG \
    virtual TagType getTag() const { return TAG; } \
    static const TagType TAG;

struct X_1 : public XA
{
    DEFTAG;

    virtual void print(std::ostream& os) const { os << "X_1(" << x_1 << ","; XA::print(os); os << ")"; }
    uint32_t x_1;
    template <typename F> void forEach(F & f)
    {
        XA::forEach<F>(f);
        f(x_1);
    }
    template <typename F> void forEach(F & f) const
    {
        XA::forEach<F>(f);
        f(x_1);
    }
};

struct X_2 : public XA
{
    DEFTAG;

    virtual void print(std::ostream& os) const { os << "X_2(" << x_2 << ","; XA::print(os); os << ")"; }
    uint32_t x_2;
    template <typename F> void forEach(F & f)
    {
        XA::forEach<F>(f);
        f(x_2);
    }
    template <typename F> void forEach(F & f) const
    {
        XA::forEach<F>(f);
        f(x_2);
    }
};

struct X_2_1 : public X_2
{
    DEFTAG;

    virtual void print(std::ostream& os) const { os << "X_2_1(" << x_2_1 << ","; X_2::print(os); os << ")"; }
    uint32_t x_2_1;
    template <typename F> void forEach(F & f)
    {
        X_2::forEach<F>(f);
        f(x_2_1);
    }
    template <typename F> void forEach(F & f) const
    {
        X_2::forEach<F>(f);
        f(x_2_1);
    }
};

struct X_2_2 : public X_2
{
    DEFTAG;

    virtual void print(std::ostream& os) const { os << "X_2_2(" << x_2_2 << ","; X_2::print(os); os << ")"; }
    uint32_t x_2_2;
    template <typename F> void forEach(F & f)
    {
        X_2::forEach<F>(f);
        f(x_2_2);
    }
    template <typename F> void forEach(F & f) const
    {
        X_2::forEach<F>(f);
        f(x_2_2);
    }
};

//tags for virtual classes
const XA::TagType X_1::TAG = 1;
const XA::TagType X_2::TAG = 2;
const XA::TagType X_2_1::TAG = 3;
const XA::TagType X_2_2::TAG = 4;
//end of virtual structures


//typedef Virtual<XA, ListElem<X_1, ListElem<X_2, ListElem<X_2_1, ListElem<X_2_2, ListHead> > > > > Virtual_XA;
typedef Virtual<XA, makeList4<X_1, X_2, X_2_1, X_2_2>::value > Virtual_XA;



//misc functions

namespace Misc
{

template <typename T>
T Rand()
{
    T x = T();
    for (size_t i=0; i<sizeof(T); i++) {
        T xr = rand() & 0xff;
        x |= (xr << (i*8));
    }
    return x;
}

}

//end of misc functions

struct MyStruct
{
    uint64_t a;
    uint32_t b;
    int16_t c;
    int32_t d;
    std::vector<char> s1;
    std::vector<uint8_t> s2;

    //Virtual_XA xa;

    template <typename F>
    void forEach(F & f)
    {
        f(a); f(b); f(c); f(d); f(s1); f(s2); //f(xa);
    }

    template <typename F>
    void forEach(F & f) const
    {
        f(a); f(b); f(c); f(d); f(s1); f(s2); //f(xa);
    }

    friend std::ostream& operator<< (std::ostream& os, const MyStruct ms)
    {
        os << '[' << ms.a << ", " << ms.b << ", " << ms.c << ", " << ms.d << ", " << "\"";
        for (std::vector<char>::const_iterator it = ms.s1.begin(); it!=ms.s1.end(); ++it)
        {
            os << *it;
        }
        os << "\", \"";
        for (std::vector<uint8_t>::const_iterator it=ms.s2.begin(); it!=ms.s2.end(); ++it)
        {
            os << *it;
        }
        //os << "\", ";
        //ms.xa.get().print(os);
        os << "\"]";
        return os;
    }

    static MyStruct Rand()
    {
        MyStruct ms;

        ms.a = Misc::Rand<uint64_t>() % 200;
        ms.b = Misc::Rand<uint32_t>() % 50;
        ms.c = Misc::Rand<int16_t>() % 30;
        ms.d = Misc::Rand<int32_t>();

        ms.s1.resize(5+rand()%10);
        for (std::vector<char>::iterator it=ms.s1.begin(); it!=ms.s1.end(); ++it) {
            *it = 'a'+(rand() % ('z'-'a'+1) );
        }
        ms.s2.resize(4);
        for (std::vector<uint8_t>::iterator it=ms.s2.begin(); it!=ms.s2.end(); ++it) {
            *it = 'a'+(rand() % ('f'-'a'+1) );
        }

        return ms;
    }
};

struct MyStruct2
{
    uint64_t id;
    std::vector<Virtual_XA> xav;

    template <typename F>
    void forEach(F & f)
    {
        f(id); f(xav);
    }

    template <typename F>
    void forEach(F & f) const 
    {
        f(id); f(xav);
    }

    friend std::ostream& operator<< (std::ostream& os, const MyStruct2& ms2)
    {
        os << "[" << ms2.id << ", (size:" << ms2.xav.size() << ")";
        for (std::vector<Virtual_XA>::const_iterator it = ms2.xav.begin(); it!=ms2.xav.end(); ++it) {
            if (it!=ms2.xav.begin()) {
                os << " ";
            }
            if (!it->get_ptr()) {
                os << "NULL";
            }
            else os << it->get();
        }
        return os;
    }

    static MyStruct2 Rand()
    {
        MyStruct2 ms2;

        ms2.id = (uint64_t)rand() + (((uint64_t)rand())<<32);

        size_t size = rand() % 20;
        ms2.xav.resize(size);
        for (std::vector<Virtual_XA>::iterator it = ms2.xav.begin(); it != ms2.xav.end(); ++it)
        {
            uint8_t type = rand() % 5;
            switch (type) {
                case 1: {
                            std::auto_ptr<X_1> x(new X_1());
                            x->xa = rand();
                            x->x_1 = rand();
                            it->set(x.release());
                            break;
                        }
                case 2: {
                            std::auto_ptr<X_2> x(new X_2());
                            x->xa = rand();
                            x->x_2 = rand();
                            it->set(x.release());
                            break;
                        }
                case 3: {
                            std::auto_ptr<X_2_1> x(new X_2_1());
                            x->xa = rand();
                            x->x_2 = rand();
                            x->x_2_1 = rand();
                            it->set(x.release());
                            break;
                        }
                case 4: {
                            std::auto_ptr<X_2_2> x(new X_2_2());
                            x->xa = rand();
                            x->x_2 = rand();
                            x->x_2_2 = rand();
                            it->set(x.release());
                            break;
                        }
            }
        }
        return ms2;
    }
};



struct SimpleTest
{
    template <typename T>
    void test()
    {
        T x = Misc::Rand<T>();
        T y_big, y_lit;

        makeTest<BigEndianCodec, T>(x, y_big, true);
        makeTest<LittleEndianCodec, T>(x, y_lit, true);

        if (x!=y_big || x!=y_lit) {
            std::cout << "mismatch for (" << typeid(T).name() << "), "
                << x << "|" << y_big << "|" << y_lit << std::endl;
        }
    }

private:
    template <template <class> class Codec, typename T>
    void makeTest(T x, T& y, bool print=false)
    {
        std::vector<char> buff;
        BufferEncoder<Codec> enc(buff);

        enc(x);

        std::vector<char>::const_iterator begin = buff.begin();
        BufferDecoder<Codec> dec(begin, buff.end());

        dec(y);
        if (print)
        {
            for (std::vector<char>::const_iterator it=buff.begin(); it!=buff.end(); ++it)
            {
                std::cout << '\\' << (uint32_t)(uint8_t)(*it);
            }
            std::cout << std::endl;
        }
    }
};

int Test0()
{
    for (size_t i=0; i<1000; i++) {
        SimpleTest().test<uint64_t>();
    }
    return 0;
}

int Test1()
{

    //simple test for MyStruct
    MyStruct ms;

    ms.a = 1;
    ms.b = 2;
    ms.c = 3;
    ms.d = 4;
    ms.s1.resize(5,'a');
    ms.s2.resize(7,'b');
    std::auto_ptr<X_2_2> ptr(new X_2_2());
    ptr->xa = 102;
    ptr->x_2 = 105;
    ptr->x_2_2 = 112;
    //ms.xa.set(ptr.release());

    std::vector<char> buff;

    BufferEncoder<BigEndianCodec> enc(buff);
    enc(ms);

    std::cout << "Buffer(" << buff.size() << ") [";
    for (std::vector<char>::const_iterator it = buff.begin(); it!=buff.end(); ++it) {
        std::cout << "\\" << (uint32_t)(uint8_t)(*it);
    }
    std::cout << "]" << std::endl;

    MyStruct ms_back;
    std::vector<char>::const_iterator begin = buff.begin();
    BufferDecoder<BigEndianCodec> dec(begin, buff.end());
    dec(ms_back);


    std::cout << ms << std::endl;
    std::cout << ms_back << std::endl;

    return 0;
}

int Test2()
{
    MyStruct2 ms2 = MyStruct2::Rand();

    std::vector<char> buff;
    BufferEncoder<BigEndianCodec> enc(buff);

    enc(ms2);

    std::cout << "Buffered in " << buff.size() << std::endl;

    MyStruct2 ms2_back;

    std::vector<char>::const_iterator begin = buff.begin();
    BufferDecoder<BigEndianCodec> dec(begin, buff.end());
    dec(ms2_back);

    std::cout << ms2 << std::endl;
    std::cout << ms2_back << std::endl;

    return 0;
}


int Test3(int argc, char **argv)
{
    if (argc<2) {
        throw "too few params?";
    }

    std::string filename("testowy.dat");

    std::vector<MyStruct> msv;
    std::vector<MyStruct2> ms2v;

    if (!strcmp(argv[1], "-write"))
    {

        for (int i=0; i<20; i++) {
            msv.push_back(MyStruct::Rand());
        }
        for (int i=0; i<10; i++) {
            ms2v.push_back(MyStruct2::Rand());
        }

        int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
        if (fd<0) {
            throw "open error";
        }

        {
            FileEncoder<BigEndianCodec> fenc(fd);

            fenc(msv);
            fenc(ms2v);
        }

        std::cout << msv << std::endl
            << ms2v << std::endl;

        if (close(fd)<0) {
            throw "close error";
        }
        return 0;
    }

    if (!strcmp(argv[1], "-read"))
    {
        int fd = open(filename.c_str(), O_RDONLY);

        {
            FileDecoder<BigEndianCodec> fdec(fd);

            fdec(msv);
            fdec(ms2v);
        }

        std::cout << msv << std::endl
            << ms2v << std::endl;

        if (close(fd)<0) {
            throw "close error";
        }
        return 0;
    }

    throw "unknown param, try -write or -read";
    //return 0;
}



template <typename T>
struct OptimizedCodec : public makeCodecWithDefaultOptimizer<LittleEndianCodec>::value<T>
{
};

template <typename T>
struct OptimizedCodec2 : public makeCodecWithDefaultOptimizer<BigEndianCodec>::value<T>
{
};

struct MyStruct3
{
    int32_t id;
    std::vector<int64_t> vi;
    std::vector<std::string> vs;

    template <typename F>
    void  forEach(F & f)
    {
        f(id); f(vi); f(vs);
    }
    template <typename F>
    void  forEach(F & f) const
    {
        f(id); f(vi); f(vs);
    }

    friend std::ostream& operator<< (std::ostream& os, const MyStruct3& ms3)
    {
        return os << "[" << ms3.id << ", " << ms3.vi << ", " << ms3.vs << "]";
    }
};


int Test4()
{
    if (false)
    {
        std::vector<char> buff;
        BufferEncoder< OptimizedCodec > enc(buff);

        int32_t x = 123456789;
        enc(x);

        std::vector<char>::const_iterator begin = buff.begin();
        BufferDecoder< OptimizedCodec > dec(begin, buff.end());
        int32_t y;
        dec(y);

        begin = buff.begin();
        BufferDecoder<LittleEndianCodec> le_dec(begin, buff.end());
        int32_t le_x;
        le_dec(le_x);

        begin = buff.begin();
        BufferDecoder<BigEndianCodec> be_dec(begin, buff.end());
        int32_t be_x;
        be_dec(be_x);

        std::cout << x << " " << y << " le(" << le_x << "), be(" << be_x << ")" << std::endl;
        return 0;
    }

    std::string filename("testowy.dat");
    {
        int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
        if (fd<0) {
            throw "open error";
        }
        MyStruct3 ms3;
        ms3.vi.push_back(145);
        ms3.vi.push_back(-7);
        ms3.vi.push_back(12345678904200LL);
        ms3.vs.push_back("hello");
        ms3.vs.push_back("world");
        ms3.vs.push_back("!!!");
        {
            FileEncoder< OptimizedCodec > fenc(fd);

            fenc(ms3);
        }
        std::cout << "encoded: " << ms3 << std::endl;

        if (close(fd)<0) {
            throw "close error";
        }
    }
    {
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd<0) {
            throw "open error";
        }

        MyStruct3 ms3_dec;
        {
            FileDecoder< OptimizedCodec > fdec(fd);

            fdec(ms3_dec);
        }
        std::cout << "decoded: " << ms3_dec << std::endl;

        if (close(fd)<0) {
            throw "close error";
        }
    }

    return 0;
}

struct TimeStats
{
    TimeStats()
    {
        getTimes(real_, user_, sys_);
    }
    ~TimeStats()
    {
        uint64_t r, u, s;
        getTimes(r, u, s);
        fprintf(stderr, "%.3f/%.3f/%.3f\n", (double)(r-real_)/1000.0, (double)(u-user_)/1000.0, (double)(s-sys_)/1000.0);
    }
private:
    void getTimes(uint64_t& real, uint64_t& user, uint64_t& sys)
    {
        struct timeval tv;
         if (gettimeofday(&tv, NULL)<0) {
            perror("gettimeofday");
        }
        struct rusage ru;
        if (getrusage(RUSAGE_SELF, &ru)<0) {
            perror("getrusage");
        }
        real = (uint64_t)tv.tv_sec*(uint64_t)1000 + (uint64_t)tv.tv_usec/(uint64_t)1000;
        user = (uint64_t)ru.ru_utime.tv_sec*(uint64_t)1000 + (uint64_t)ru.ru_utime.tv_usec/(uint64_t)1000;
        sys = (uint64_t)ru.ru_stime.tv_sec*(uint64_t)1000 + (uint64_t)ru.ru_stime.tv_usec/(uint64_t)1000;
    }
private:
    uint64_t real_;
    uint64_t user_;
    uint64_t sys_;
};

int Test5(int argc, char **argv)
{
    if (argc!=3) {
        std::cerr << "Usage: " << argv[0] << ' ' << "write|read le|ole" << std::endl;
        return 1;
    }

    std::string filename("testowy.dat");

    std::string cmd = argv[1];
    std::string codec = argv[2];


    if (cmd=="write") {
        //write ~400MB
        std::vector<std::string> vs;
        {
            TimeStats ts;
            vs.reserve(1024);
            boost::minstd_rand0 random(13);
            for (size_t i=0; i<1024*40; i++) {
                std::string ns;
                ns.resize(random()%(1024*20+1));
                for (std::string::iterator it=ns.begin(); it!=ns.end(); it++) {
                    *it = 'a' + random() % ('z'-'a'+1);
                }
                vs.push_back(ns);
            }
        }
        int fd = open(filename.c_str(), O_CREAT|O_WRONLY, 0644);
        if (fd<0) {
            throw "open error";
        }

        if (codec=="le") {
            TimeStats ts;
            FileEncoder<LittleEndianCodec> fenc(fd);
            fenc(vs);
        }
        if (codec=="ole") {
            TimeStats ts;
            FileEncoder<OptimizedCodec> fenc(fd);
            fenc(vs);
        }
        if (close(fd)<0) {
            throw "close error";
        }
        return 0;
    }
    if (cmd=="read") {
        std::vector<std::string> vs;

        int fd = open(filename.c_str(), O_RDONLY);
        if (fd<0) {
            throw "open error";
        }
        if (codec=="le") {
            TimeStats ts;
            FileDecoder<LittleEndianCodec> fdec(fd);
            fdec(vs);
        }
        if (codec=="ole") {
            TimeStats ts;
            FileDecoder<OptimizedCodec> fdec(fd);
            fdec(vs);
        }
        if (close(fd)<0) {
            throw "close error";
        }
        return 0;
    }

    throw "unknown param";
    //return 0;
}


int main(int argc, char **argv)
    try
{
    //return Test0();
    //return Test1();
    //return Test2();
    //return Test3(argc, argv);
    //return Test4();

    return Test5(argc, argv);
    
    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
catch (const char * c) {
    std::cerr << c << std::endl;
    return 1;
}
