#include <cstdlib>
#include <fstream>
#include <iostream>

#include "Parser.hpp"

// Use tutorials in: https://llvm.org/docs/tutorial/

int main (int argc, char *argv[])
{

    std::cout << "LL1Syntactic analyzer" << std::endl;
    std::cout << "---------------------" << std::endl;

    std::ifstream ifs;
    bool res;

    if (argc == 2) {
        ifs.open(argv[1]);
        if (!ifs) {
            std::cout << "File " << argv[1] << " could not be opened" << std::endl;
            return EXIT_FAILURE;
        }
        res = true;
    } 
    else {
        res = false;
    }

    Parser parser;
    parser.initLexer(ifs);
    if (!parser.Parse()) {
        return 1;
    }

    parser.Generate().print(llvm::outs(), nullptr);

    return 0;
}
