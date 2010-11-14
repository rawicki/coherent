#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "gen_test.h"


int main(int argc, char **argv)
{
    std::vector<char> buffer;

    checkBaseTree();

    VirtualBase vb;
    fun(vb, buffer);

    std::cout << buffer.size() << std::endl;

    return 0;
}

