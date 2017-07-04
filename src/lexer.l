%{
#include "src/Parser.hxx"
%}

%%
[[:space:]]+
"//".*

"("           return yy::parser::make_LPAREN();
")"           return yy::parser::make_RPAREN();
"+"           return yy::parser::make_PLUS();
"*"           return yy::parser::make_STAR();
"="           return yy::parser::make_EQUAL();
"==="         return yy::parser::make_UNIFY();
"$"           return yy::parser::make_EVAL();
"`"           return yy::parser::make_BACKTICK();
"||"          return yy::parser::make_OR();
"&&"          return yy::parser::make_AND();
"=="          return yy::parser::make_EQUALEQUAL();
"!="          return yy::parser::make_NOTEQUAL();
"<"           return yy::parser::make_LESS();
">"           return yy::parser::make_GREATER();
"<="          return yy::parser::make_LESSEQUAL();
">="          return yy::parser::make_GREATEREQUAL();
"-"           return yy::parser::make_MINUS();
"/"           return yy::parser::make_SLASH();
"%"           return yy::parser::make_PROCENT();
"!"           return yy::parser::make_NOT();
"&"           return yy::parser::make_AMPERSAND();


"struct"      return yy::parser::make_STRUCT();
"{"           return yy::parser::make_LBRACE();
"}"           return yy::parser::make_RBRACE();
";"           return yy::parser::make_SEMICOLON();
","           return yy::parser::make_COMMA();
"void"        return yy::parser::make_VOID();
":-"          return yy::parser::make_DECLARATIVE_DEFINE();
"."           return yy::parser::make_DOT();
"break"       return yy::parser::make_BREAK();
"continue"    return yy::parser::make_CONTINUE();
"if"          return yy::parser::make_IF();
"else"        return yy::parser::make_ELSE();
"for"         return yy::parser::make_FOR();
"extern"      return yy::parser::make_EXTERN();
"\"C\""       return yy::parser::make_C_STRING();

"true"        return yy::parser::make_TRUE();
"false"       return yy::parser::make_FALSE();
[[:alpha:]]+  return yy::parser::make_ID(yytext);
[[:digit:]]+  return yy::parser::make_DECIMAL(yytext);

<<EOF>>       return yy::parser::make_END();

%%
