%{

#include <stdio.h>
#include <iostream>
#include <string>

#include "AST.h"

extern FILE *yyin;

extern int yylex(void);
void yyerror(char *);

AST::Prog *Root;

%}

%union {
    int iVal;
    AST::Prog *prog;
    AST::Stmt *stmt;
    AST::Stmts *stmts;
    AST::Expr *expr;
}

%token SEMI ADD RET

%token<iVal>		INTEGER

%type<prog>		Prog
%type<stmt>		Stmt
%type<stmts>		Stmts
%type<expr>		Expr


%start  Prog

%left   ADD

%%

Prog: Stmts { $$ = new AST::Prog($1); Root = $$; }

Stmts: Stmts Stmt { $$ = $1; $$->push_back($2); }
       | { $$ = new AST::Stmts(); }

Stmt: Expr SEMI { $$ = $1; }

Expr: Expr ADD Expr { $$ = new AST::AddExpr($1, $3); }
      | INTEGER { $$ = new AST::Integer($1); }


%%

void yyerror(char *str) {
    std::cout << "Error: " << str << std::endl;
    exit(1);
}

//int main(int argc, char **argv) {
//    yyin = fopen(argv[1], "r");
//    yyparse();
//    fclose(yyin);
//}
