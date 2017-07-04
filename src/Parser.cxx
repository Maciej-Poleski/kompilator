#include "Parser.hxx"

#include "lexer.hxx"

int Parser::parse(const std::string &f)
{
    yyin = fopen(f.c_str(), "r");
    yy::parser parser(*this);
    int res = parser.parse();
    return res;
}

void yy::parser::error(const std::string &msg)
{
    std::cerr << "ERROR: " << msg << '\n';
}