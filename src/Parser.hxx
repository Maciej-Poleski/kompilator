//
// Created by local on 05.02.17.
//

#ifndef KOMPILATOR_DRIVER_HXX
#define KOMPILATOR_DRIVER_HXX

#include "parser.hxx"

#define YY_DECL yy::parser::symbol_type yylex (Parser& )
YY_DECL;

class Parser
{
    friend yy::parser;
public:
    int parse (const std::string& f);

//private:
    std::shared_ptr<Module> result;
};


#endif //KOMPILATOR_DRIVER_HXX
