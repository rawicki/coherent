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
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <boost/lexical_cast.hpp>


struct Indent
{
    Indent(size_t x) : x_(x)
    {
    }
    friend std::ostream& operator<< (std::ostream& os, const Indent& ind)
    {
        for (size_t i=0; i<ind.x_; i++)
            os << ' ';
        return os;
    }
    size_t x_;
};

void makeTreeFor(size_t left, size_t right, std::ostream& os, size_t level)
{
    //if (left>right) {
    //    os << "tree_leaf";
    //    return;
    //}

    //size_t curr = left+(right-left)/2;

    //os << std::endl << Indent(level) << "/*"<<(level/2-1)<<"*/ ";
    //os << "tree_node<Foo_" << curr << ", ";
    //    makeTreeFor(left, curr-1, os, level+2);
    //os << ',';
    //    makeTreeFor(curr+1, right, os, level+2);
    //os << "> ";

    if (left>right) {
        return;
    }
    size_t curr = left+(1+right-left)/2;

    makeTreeFor(left, curr-1, os, level+2);
    makeTreeFor(curr+1, right, os, level+2);

    os << Indent(level) << "/*" << (level/2-1) << "*/" << " typedef tree_node<" << "Foo_" << curr << ", ";
    if (left<curr) os << "tree_node_" << left << "_" << (curr-1); else os << "tree_leaf";
    os << ", ";
    if (curr<right) os << "tree_node_" << (curr+1) << "_" << right; else os << "tree_leaf";
    os << "> tree_node_" << left << "_" << right<< ";" << std::endl;
}


int main(int argc, char **argv)
{
    assert(argc>1);

    size_t n = boost::lexical_cast<size_t>(argv[1]);

    if (argc>2 && !strcmp(argv[2], "static"))
    {
        std::cout
            << "#include \"codecs/little_endian.h\"" << std::endl
            << "#include \"encoders/buffer_encoder.h\"" << std::endl
            << "#include \"gen_test.h\"" << std::endl
            << std::endl
            << "using namespace coherent::serialization;" << std::endl
            << std::endl
            << std::endl;
        for (size_t cid=1; cid<=n; cid++)
        {
            std::cout << "const Base::tag_type Foo_" << cid << "::TAG = " << cid << ";" << std::endl;
        }
        std::cout
            << std::endl
            << std::endl;
        std::cout
            << "void check_base_tree()" << std::endl
            << "{" << std::endl
            << "    check_tree<BaseTree>();" << std::endl
            << "}" << std::endl
            << std::endl;
        std::cout
            << "void fun(VirtualBase x, std::vector<char>& buffer)" << std::endl
            << "{" << std::endl
            << "    buffer_encoder<little_endian_codec> enc(buffer);" << std::endl
            << "    enc(x);" << std::endl
            << "}" << std::endl
            << std::endl;
        return 0;
    }

    std::vector<std::string> types;
    types.push_back("int8_t");
    types.push_back("uint16_t");
    types.push_back("std::vector<char>");
    types.push_back("std::string");
    types.push_back("std::pair<uint32_t, uint32_t>");

    std::cout
        << "#ifndef GEN_TEST_H" << std::endl
        << "#define GEN_TEST_H" << std::endl
        << std::endl;
        
    std::cout
        << "#include \"misc/virtual_class.h\"" << std::endl
        << std::endl
        << "using namespace coherent::misc;" << std::endl
        << std::endl
        << std::endl;
    //base
    std::cout
        << "struct Base" << std::endl
        << "{" << std::endl
        << "    typedef uint16_t tag_type;" << std::endl
        << "    virtual tag_type get_tag() const = 0;" << std::endl
        << "    virtual ~Base() {}" << std::endl
        << "    uint32_t base_;" << std::endl
        << "    template <typename F> void for_each(F & f) { f(base_); }" << std::endl
        << "    template <typename F> void for_each(F & f) const { f(base_); }" << std::endl
        << "};" << std::endl
        << std::endl;

    srand(7);
    for (size_t cid=1; cid<=n; cid++)
    {
        size_t base = rand() % cid;
        std::cout
            << "struct Foo_" << cid << " : public ";
        if (base!=0)
            std::cout << "Foo_" << base;
        else
            std::cout << "Base";
        std::cout << std::endl
            << "{" << std::endl
            << "    typedef ";
        if (base!=0)
            std::cout << "Foo_" << base;
        else
            std::cout << "Base";
        std::cout << " Super;" << std::endl
            << "    virtual tag_type get_tag() const { return TAG; }" << std::endl
            << "    static const tag_type TAG;" << std::endl;
        size_t field = rand() % types.size();
        std::cout
            << "    " << types[field] << " field" << cid << "_;" << std::endl
            << "    template <typename F> void for_each(F & f) { Super::for_each<F>(f); f(field" << cid << "_); }" << std::endl
            << "    template <typename F> void for_each(F & f) const { Super::for_each<F>(f); f(field" << cid << "_); }" << std::endl
            << "};" << std::endl
            << std::endl;
        //std::cout <<
        //    "const Base::tag_type Foo_" << cid << "::TAG = " << cid << ";" << std::endl;
    }
    std::cout << std::endl;



    /*std::cout << "typedef Virtual<Base, " << std::endl;
    for (size_t cid=1; cid<=n; cid++)
    {
        for (size_t i=0;i<=cid; i++) std::cout << ' ';
        std::cout << "ListElem<" << "Foo_" << cid << ", " << std::endl;
    }
    for (size_t i=0; i<=n+1; i++) std::cout << ' ';
    std::cout << "ListHead" << std::endl;
    for (size_t cid=n; cid>=1; cid--) {
        for (size_t i=0; i<=cid; i++) std::cout << ' ';
        std::cout << ">" << std::endl;
    }
    std::cout << "> VirtualBase;" << std::endl;
    std::cout << std::endl;*/



    /*std::cout << "typedef ListElem<Foo_" << n << ", ListHead> Node_" << n << ";" << std::endl;
    for (size_t cid=n-1; cid>=1; cid--)
    {
        std::cout << "typedef ListElem<Foo_" << cid << ", Node_" << (cid+1) << "> Node_" << cid << ";" << std::endl;
    }
    std::cout << "typedef Virtual<Base, Node_1> VirtualBase;" << std::endl;
    std::cout << std::endl;*/

//    std::cout << "typedef ";
    makeTreeFor(1, n, std::cout, 2);
//    std::cout << std::endl;
//    std::cout << "BaseTree;" << std::endl;
    std::cout << "typedef tree_node_1_" << n << " BaseTree;" << std::endl;
    std::cout << "typedef Virtual<Base, BaseTree> VirtualBase;" << std::endl;
    std::cout << std::endl;

    std::cout
        << "void fun(VirtualBase x, std::vector<char>& buffer);" << std::endl
        << "void check_base_tree();" << std::endl;
//        << "{" << std::endl
//        << "    buffer_encoder<little_endian_codec> enc(buffer);" << std::endl
//        << "    enc(x);" << std::endl
//        << "}" << std::endl
//        << std::endl;

    std::cout << std::endl;
    std::cout << "#endif /* GEN_TEST_H */" << std::endl;

    return 0;
}

