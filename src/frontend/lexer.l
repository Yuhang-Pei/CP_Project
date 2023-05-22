%{

#include <stdio.h>

#include "AST.h"
#include "parser.hpp"

%}

%option noyywrap

%%

";"                     { return SEMI; }
","                     { return COMMA; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"["                     { return LBRACKET; }
"]"                     { return RBRACKET; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"=="                    { return EQUAL; }
"+"		                { return ADD; }
"="                     { return ASSIGN; }
"return"                { return RETURN; }
"int"                   { return INT; }
"void"                  { return VOID; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.identifier = new std::string(yytext, yyleng); return IDENTIFIER; }
[1-9][0-9]*|0           { sscanf(yytext, "%d", &(yylval.intVal)); return INTEGER; }
[ \n\t]+                ;
.

%%