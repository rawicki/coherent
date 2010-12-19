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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <inttypes.h>
#include <assert.h>
#include <boost/static_assert.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "codecs/little_endian.h"
#include "encoders/buffer_encoder.h"
#include "misc/virtual_class.h"


struct A
{
    typedef int32_t TagType;
    virtual TagType getTag() const = 0;

    virtual ~A() {}

    virtual std::ostream& print(std::ostream&) const = 0;

    friend std::ostream& operator<< (std::ostream& os, const A& a)
    {
        return a.print(os);
    }
};

struct A1 : public A
{
    enum { TAG = 1 };
    virtual TagType getTag() const
    {
        return TAG;
    }

    virtual std::ostream& print(std::ostream& os) const
    {
        return os << "A1(" << a1_ << ")";
    }

    A1() {}
    A1(const std::string& s) : a1_(s) {}

    template <typename F>
    void forEach(F & f)
    {
        f(a1_);
    }
    template <typename F>
    void forEach(F & f) const
    {
        f(a1_);
    }


    std::string a1_;
};

struct A2 : public A
{
    enum { TAG = 2 };
    virtual TagType getTag() const
    {
        return TAG;
    }

    virtual std::ostream& print(std::ostream& os) const
    {
        return os << "A2(" << a2_ << ")";
    }

    A2() {}
    A2(uint64_t x) : a2_(x) {}

    template <typename F>
    void forEach(F & f)
    {
        f(a2_);
    }
    template <typename F>
    void forEach(F & f) const
    {
        f(a2_);
    }

    uint64_t a2_;
};





struct ACont
{
    boost::shared_ptr<A> a_;    //check simple A*

    typedef TreeNode<A2, CreateTreeNode<A1>::value, TreeLeaf> VTree;
    //typedef TreeNode<A1, CreateTreeNode<A2>::value, TreeLeaf> BadVTree;

    typedef makeList2<A1,A2>::value VList;
    typedef Virtual<A, VTree> VirtualA;
    typedef VirtualA::Helper VirtualAH;

    BOOST_STATIC_ASSERT( checker_detail::TreeChecker<ACont::VTree>::ok );
    //BOOST_STATIC_ASSERT( checker_detail::TreeChecker<ACont::BadVTree>::ok );

    template <typename F>
    void forEach(F & f)
    {
        VirtualAH(f, a_);
    }
    template <typename F>
    void forEach(F & f) const
    {
        VirtualAH(f, a_);
    }

    friend std::ostream& operator<< (std::ostream& os, const ACont& ac)
    {
        return (ac.a_.get()) ? (os << *(ac.a_)) : (os << "null");
    }
};

struct AContPtr
{
    A * a_;

    typedef Virtual<A, ACont::VTree, StdPtrPolicy<A, A*> >::Helper VirtualAH;

    AContPtr() : a_(NULL)
    {
    }
    ~AContPtr()
    {
        if (a_) {
            delete a_;
        }
    }

    template <typename F>
    void forEach(F & f)
    {
        VirtualAH(f, a_);
    }
    template <typename F>
    void forEach(F & f) const
    {
        VirtualAH(f, a_);
    }

    friend std::ostream& operator<< (std::ostream& os, const AContPtr& ac)
    {
        return (ac.a_==NULL) ? (os << "null") : (os << *(ac.a_));
    }
};



struct AFactory
{
    template <typename F>
    struct Factory
    {
        struct CallBacks
        {
            typedef boost::function<void(F&)> CB;

            CallBacks()
            {
                cbs_.resize(3);
                cbs_[1] = boost::bind(& F::template processChecked<A1>, _1);
                cbs_[2] = boost::bind(& F::template processChecked<A2>, _1);
            }
            void go(F & f) const
            {
                std::cout << "callback: " << (int32_t)f.tag_
                    << " using: " << this << ", cbs: " << cbs_.size() << std::endl;
                assert(0<f.tag_ && f.tag_<3);
                cbs_[f.tag_] (f);
            }
            std::vector<CB> cbs_;
        };

        static const CallBacks callbacks;

        void operator() (F & f) const
        {
            callbacks.go(f);
        }
    };
};

template <typename F> const typename AFactory::Factory<F>::CallBacks AFactory::Factory<F>::callbacks;
//readelf -W -a vc_example | c++filt


struct AContFact
{
    boost::shared_ptr<A> a_;

    typedef Virtual<A, ObjectFactory<AFactory> > VirtualA;
    typedef VirtualA::Helper VirtualAH;

    template <typename F>
    void forEach(F & f)
    {
        VirtualAH(f, a_);
    }
    template <typename F>
    void forEach(F & f) const
    {
        VirtualAH(f, a_);
    }

    friend std::ostream& operator<< (std::ostream& os, const AContFact& ac)
    {
        return (ac.a_.get()) ? (os << *(ac.a_)) : (os << "null");
    }
};


struct PrintBuf
{
    const std::vector<char>& buf_;

    PrintBuf(const std::vector<char>& buf) : buf_(buf)
    {
    }
    friend std::ostream& operator<< (std::ostream& os, const PrintBuf& pb)
    {
        const std::vector<char>& buf = pb.buf_;
        for (std::vector<char>::const_iterator it=buf.begin(); it!=buf.end(); ++it)
        {
            uint8_t c = *it;
            if (32<=c && c<127)
                os << (char)c;
            else
                os << '\\'
                    << (char)((c/16>9) ? (c/16-10+'a') : (c/16+'0'))
                    << (char)((c%16>9) ? (c%16-10+'a') : (c%16+'0'));
        }
        return os;
    }
};


int Main()
{

    assert( checker_detail::TreeChecker<ACont::VTree>::ok );

    std::vector<char> buf;

    {
        //encode some objects
        ACont ac1;
        ac1.a_.reset(new A1("obiekt klasy A1"));

        ACont ac2;
        ac2.a_.reset(new A2(777));

        ACont ac3;

        BufferEncoder<LittleEndianCodec> enc(buf);

        enc(ac1)('x')(ac2)(ac3);

        std::cout << ac1 << " x " << ac2 << " " << ac3 << std::endl;
    }

    std::cout << PrintBuf(buf) << std::endl;

    {
        //decoder using ACont - default vc with boost::shared_ptr<A>
        std::vector<char>::const_iterator it = buf.begin();
        BufferDecoder<LittleEndianCodec> dec(it, buf.end());

        ACont ac1;
        ACont ac2;
        ACont ac3;
        char p;

        dec(ac1)(p)(ac2)(ac3);

        std::cout << ac1 << " " << p << " " << ac2 << " " << ac3 << std::endl;
    }

    {
        //decoder using AcontPtr - vc with changed ptr policy = A*
        std::vector<char>::const_iterator it = buf.begin();
        BufferDecoder<LittleEndianCodec> dec(it, buf.end());

        AContPtr ac1;
        AContPtr ac2;
        AContPtr ac3;
        char p;

        dec(ac1)(p)(ac2)(ac3);

        std::cout << ac1 << " " << p << " " << ac2 << " " << ac3 << std::endl;
    }

    {
        //example using object factory
        AContFact ac0;
        ac0.a_.reset(new A1("czwarty obiekt"));

        BufferEncoder<LittleEndianCodec> enc(buf);
        enc(ac0);

        std::vector<char>::const_iterator it = buf.begin();
        BufferDecoder<LittleEndianCodec> dec(it, buf.end());

        AContFact ac1;
        AContFact ac2;
        AContFact ac3;
        AContFact ac4;
        char p;

        dec(ac1)(p)(ac2)(ac3)(ac4);

        std::cout << ac1 << " " << p << " " << ac2 << " " << ac3 << " " << ac4 << std::endl;
    }

    return 0;
}


int main()
    try
{
    return Main();
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
}
catch (const char * e) {
    std::cerr << e << std::endl;
    return 1;
}
