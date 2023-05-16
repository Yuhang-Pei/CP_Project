%{

#include <cstdio>
#include <cstdlib>

#include "AST.h"

extern int yylex(void);
void yyerror(const char *str) {
    std::cout << "Error: " << str << std::endl;
    exit(1);
}

AST::Prog *Root;

%}

%union {
    int iVal;
    AST::Prog *prog;
    AST::Node *node;
    AST::Stmt *stmt;
    AST::Stmts *stmts;
    AST::Expr *expr;
}

%token<iVal>	INTEGER
%token<token>	SEMI ADD

%type<prog>	Prog
%type<stmt>	Stmt
%type<stmts>	Stmts
%type<expr>	Expr

%left   ADD

%start  Prog

%%

Prog: Stmts { $$ = new AST::Prog($1); Root = $$; }

Stmts: Stmts Stmt { $1->push_back($2); }
       | Stmt { $$ = new AST::Stmts(); $$->push_back($1); }

Stmt: Expr SEMI { $$ = new AST::ExprStmt($1); }

Expr: Expr ADD Expr { $$ = new AST::AddExpr($1, $3); }
      | INTEGER { $$ = new AST::Integer($1); }


%%