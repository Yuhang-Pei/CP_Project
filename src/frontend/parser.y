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
    int intVal;
    std::string *identifier;

    AST::Node *node;

    AST::Prog *prog;

    AST::Unit *unit;
    AST::Units *units;

    AST::Def *def;
    AST::FuncDef *funcDef;
    AST::Param *param;
    AST::Params *params;

    AST::TypeSpecifier *typeSpecifier;
    AST::BuiltInType *builtInType;

    AST::Stmt *stmt;
    AST::Stmts *stmts;
    AST::Block *block;
    AST::ExprStmt *exprStmt;

    AST::Expr *expr;
    AST::FuncCall *funcCall;
    AST::Args *args;
    AST::AddExpr *addExpr;
    AST::Constant *constant;
    AST::Integer *integer;
}

%token<intVal>		INTEGER
%token<identifier>	IDENTIFIER
%token<token>		SEMI COMMA LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token<token>		ADD
%token<token>		INT VOID

%type<prog>		Prog
%type<unit>		Unit
%type<units>		Units
%type<def>		Def
%type<funcDef>		FuncDef
//%type<param>		Param
%type<params>		Params
%type<typeSpecifier>	TypeSpecifier
%type<builtInType>	BuiltInType
%type<stmt>		Stmt
%type<stmts>		Stmts
%type<block>		Block
//%type<exprStmt>		ExprStmt
%type<expr>		Expr
%type<funcCall>		FuncCall
%type<args>		Args
//%type<addExpr>		AddExpr
//%type<constant>		Constant
//%type<integer>		Integer

%left   ADD

%start  Prog

%%

Prog : Units { $$ = new AST::Prog($1); Root = $$; }

Units : Units Unit { $$ = $1; $$->push_back($2); }
      | Unit       { $$ = new AST::Units(); $$->push_back($1); }

Unit : Def { $$ = $1; }

Def : FuncDef { $$ = $1; }

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN Block { $$ = new AST::FuncDef($1, *$2, $4, $6); }

TypeSpecifier : BuiltInType { $$ = $1; }

BuiltInType : VOID { $$ = new AST::BuiltInType(AST::BuiltInType::_VOID); }
            | INT { $$ = new AST::BuiltInType(AST::BuiltInType::_INT); }

Params : VOID { $$ = new AST::Params(); }

Block : LBRACE Stmts RBRACE { $$ = new AST::Block($2); }

Stmts : Stmts Stmt { $$ = $1; $$->push_back($2); }
      | Stmt { $$ = new AST::Stmts(); $$->push_back($1); }

Stmt : Expr SEMI { $$ = new AST::ExprStmt($1); }

Expr : FuncCall { $$ = $1; }
     | Expr ADD Expr { $$ = new AST::AddExpr($1, $3); }
     | INTEGER { $$ = new AST::Integer($1); }

FuncCall : IDENTIFIER LPAREN Args RPAREN { $$ = new AST::FuncCall(*$1, $3); }

Args : Args COMMA Expr { $$ = $1; $$->push_back($3); }
     | Expr { $$ = new AST::Args(); $$->push_back($1); }

%%