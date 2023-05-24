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

void GetString() {
    yylval.strVal = new std::string();
    for (int i = 1; i < yyleng - 1; ++i)
        if (yytext[i] == '\\')
            yylval.strVal->push_back(Escape(yytext[++i]));
        else
            yylval.strVal->push_back(yytext[i]);
}

%}

%option noyywrap

%%

";"                     { return SEMI; }
","                     { return COMMA; }
"."                     { return DOT; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"["                     { return LBRACKET; }
"]"                     { return RBRACKET; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"=="                    { return EQUAL; }
"!="                    { return NEQ; }
">"                     { return GREAT; }
"<"                     { return LESS; }
"+"		                { return ADD; }
"*"		                { return MUL; }
"-"                     { return SUB; }
"/"                     { return DIV; }
"="                     { return ASSIGN; }
"!"                     { return NOT; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"for"                   { return FOR; }
"return"                { return RETURN; }
"void"                  { return VOID; }
"bool"                  { return BOOL; }
"char"                  { return CHAR; }
"int"                   { return INT; }
"double"                { return DOUBLE;}
"true"                  { return TRUE; }
"false"                 { return FALSE; }
"NULL"                  { return NULLPTR; }
"nullptr"               { return NULLPTR; }
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.identifier = new std::string(yytext, yyleng); return IDENTIFIER; }
[1-9][0-9]*|0           { sscanf(yytext, "%d", &(yylval.intVal)); return INTEGER; }
[0-9]+\.[0-9]+          { sscanf(yytext, "%lf", &(yylval.doubleVal)); return REAL; }
\.[0-9]+                { sscanf(yytext, "%lf", &(yylval.doubleVal)); return REAL; }
[0-9]+\.                { sscanf(yytext, "%lf", &(yylval.doubleVal)); return REAL; }
[ \n\t]+                ;
"\'"\\."\'"             { yylval.charVal = Escape(yytext[2]); return CHARACTER; }
"\'"[^\\']"\'"          { yylval.charVal = yytext[1]; return CHARACTER; }
"\""(\\.|[^\\"])*"\""   { GetString(); return STRING; }
.

%%