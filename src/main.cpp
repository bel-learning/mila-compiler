#include <cstdlib>
#include <fstream>
#include <iostream>

#include "Parser.hpp"

// Use tutorials in: https://llvm.org/docs/tutorial/

int main (int argc, char *argv[])
{

//    std::cout << "LL1Syntactic analyzer" << std::endl;
//    std::cout << "---------------------" << std::endl;
    Parser parser;
    if (!parser.Parse()) {
        return 1;
    }

    parser.Generate().print(llvm::outs(), nullptr);

    return 0;
}
