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

#include "Codecs/LittleEndianCodec.h"
#include "Encoders/BufferEncoder.h"
#include "Misc/VirtualClass.h"


struct A
{
    typedef int32_t TagType;
    virtual TagType getTag() const = 0;

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

    A1()
    {
    }

    A1(const std::string& s) : a1_(s)
    {
    }

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

    A2()
    {
    }

    A2(uint64_t x) : a2_(x)
    {
    }

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

    BOOST_STATIC_ASSERT( checker_detail::TreeChecker<ACont::VTree>::ok );
    //BOOST_STATIC_ASSERT( checker_detail::TreeChecker<ACont::BadVTree>::ok );

    template <typename F>
    void forEach(F & f)
    {
        VirtualA va;
        f(va);
        a_ = va.get_sptr();
    }
    template <typename F>
    void forEach(F & f) const
    {
        f(VirtualA(a_));
    }

    friend std::ostream& operator<< (std::ostream& os, const ACont& ac)
    {
        return os << *(ac.a_);
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
        ACont ac1;
        ac1.a_.reset(new A1("obiekt klasy A1"));

        ACont ac2;
        ac2.a_.reset(new A2(777));

        BufferEncoder<LittleEndianCodec> enc(buf);

        enc(ac1);
        enc('x');
        enc(ac2);

        std::cout << ac1 << " x " << ac2 << std::endl;
    }

    std::cout << PrintBuf(buf) << std::endl;

    {
        std::vector<char>::const_iterator it = buf.begin();
        BufferDecoder<LittleEndianCodec> dec(it, buf.end());

        ACont ac1;
        ACont ac2;
        char p;

        dec(ac1);
        dec(p);
        dec(ac2);

        std::cout << ac1 << " " << p << " " << ac2 << std::endl;
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
