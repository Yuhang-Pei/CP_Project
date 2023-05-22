%{

#include <stdio.h>

#include "AST.h"
#include "parser.hpp"

char Escape(char c) {
    switch(c) {
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case 'v':  return '\v';
        case '\\': return '\\';
        case '\?': return '\?';
        case '\'': return '\'';
        case '\"': return '\"';
        case '0':  return '\0';
        default:   return c;
    }
}

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
"void"                  { return VOID; }
"char"                  { return CHAR; }
"int"                   { return INT; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.identifier = new std::string(yytext, yyleng); return IDENTIFIER; }
[1-9][0-9]*|0           { sscanf(yytext, "%d", &(yylval.intVal)); return INTEGER; }
[ \n\t]+                ;
"\'"\\."\'"             { yylval.charVal = Escape(yytext[2]); return CHARACTER; }
"\'"[^\\']"\'"          { yylval.charVal = yytext[1]; return CHARACTER; }
.

%%