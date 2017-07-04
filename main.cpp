#include <iostream>

#include "src/Parser.hxx"
#include "src/Compiler.hxx"

void die(const char *msg)
{
    std::cerr << msg << '\n';
    exit(1);
}

int main()
{
    Parser parser;
    if (parser.parse("../sample/extern.di")) {
        die("PARSE ERROR");
    }
    if (!parser.result) {
        die("NO PARSE TREE");
    }
    Compiler compiler(parser.result, "extern");
    compiler.compile();
    return 0;
}
