%{

#include <stdio.h>

#include "parser.hpp"
#include "AST.h"

extern "C" int yywrap() { return 1; }

%}

%option never-interactive

INTEGER [1-9][0-9]*|0

%%

";"         return SEMI;
"+"		    return ADD;
[\n]        return RET;
{INTEGER}   { sscanf(yytext, "%d", &(yylval.iVal)); return INTEGER; }
[ \t]+      ;
.

%%