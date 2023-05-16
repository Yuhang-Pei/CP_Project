%{

#include <stdio.h>

#include "AST.h"
#include "parser.hpp"

%}

%option noyywrap

%%

";"             return SEMI;
"+"		        return ADD;
[1-9][0-9]*|0   { sscanf(yytext, "%d", &(yylval.iVal)); return INTEGER; }
[ \n\t]+        ;
.

%%