#include <iostream>
#include <cstring>

#include "src/Parser.hxx"
#include "src/Compiler.hxx"

void die(const char *msg)
{
    std::cerr << msg << '\n';
    exit(1);
}

void usage(char **argv)
{
    std::cerr << argv[0] << " SOURCE_FILE [-v]\n";
    exit(2);
}

int main(int argc, char **argv)
{
    std::string filename;
    bool dumpLlvm = false;
    if (argc < 2) {
        usage(argv);
    } else {
        filename = argv[1];
        if (argc == 3 && strncmp(argv[2], "-v", 2) == 0) {
            dumpLlvm = true;
        } else if (argc >= 3) {
            usage(argv);
        }
    }
    Parser parser;
    if (parser.parse(argv[1])) {
        die("PARSE ERROR");
    }
    if (!parser.result) {
        die("NO PARSE TREE");
    }

    Compiler compiler(parser.result, filename, dumpLlvm);
    compiler.compile();
    return 0;
}
