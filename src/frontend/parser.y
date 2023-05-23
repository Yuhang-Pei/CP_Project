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
    char charVal;
    int intVal;
    std::string *strVal;
    std::string *identifier;

    AST::Node *node;

    AST::Prog *prog;

    AST::Unit *unit;
    AST::Units *units;

    AST::Def *def;
    AST::FuncDef *funcDef;
    AST::Param *param;
    AST::Params *params;
    AST::FuncBody *funcBody;
    AST::VarDef *varDef;
    AST::VarInit *varInit;
    AST::VarInitList *varInitList;

    AST::TypeSpecifier *typeSpecifier;
    AST::BuiltInType *builtInType;

    AST::Stmt *stmt;
    AST::Stmts *stmts;
    AST::Block *block;
    AST::ExprStmt *exprStmt;
    AST::IfStmt *ifStmt;
    AST::ReturnStmt *returnStmt;

    AST::Expr *expr;
    AST::FuncCall *funcCall;
    AST::Args *args;
    AST::AddExpr *addExpr;
    AST::EqExpr *eqExpr;
    AST::Variable *variable;
    AST::Constant *constant;
    AST::Boolean *boolean;
    AST::Character *character;
    AST::Integer *integer;
}

%token<token>		TRUE FALSE
%token<charVal>		CHARACTER
%token<intVal>		INTEGER
%token<strVal>		STRING
%token<identifier>	IDENTIFIER
%token<token>		SEMI COMMA LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token<token>		ADD
%token<token>		EQUAL
%token<token>		ASSIGN
%token<token>		VOID BOOL CHAR INT
%token<token>		IF ELSE RETURN

%type<prog>		Prog

%type<unit>		Unit
%type<units>		Units

%type<def>		Def
%type<funcDef>		FuncDef
%type<param>		Param
%type<params>		Params
%type<funcBody>		FuncBody
%type<varDef>		VarDef
%type<varInit>		VarInit
%type<varInitList>	VarInitList

%type<typeSpecifier>	TypeSpecifier
%type<builtInType>	BuiltInType

%type<stmt>		Stmt
%type<stmts>		Stmts
%type<block>		Block
%type<ifStmt>		IfStmt
%type<returnStmt>	ReturnStmt

%type<expr>		Expr
%type<funcCall>		FuncCall
%type<args>		Args
//%type<variable>		Variable
%type<constant>		Constant

%left   ADD

%start  Prog

%%

Prog : Units { $$ = new AST::Prog($1); Root = $$; }

Units : Units Unit { $$ = $1; $$->push_back($2); }
      | Unit { $$ = new AST::Units(); $$->push_back($1); }

Unit : Def { $$ = $1; }

Def : FuncDef { $$ = $1; }
    | VarDef { $$ = $1; }

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN FuncBody { $$ = new AST::FuncDef($1, *$2, $4, $6); }

FuncBody : LBRACE Stmts RBRACE { $$ = new AST::FuncBody($2); }

VarDef : TypeSpecifier VarInitList SEMI { $$ = new AST::VarDef($1, $2); }

VarInitList : VarInitList COMMA VarInit { $$ = $1; $$->push_back($3); }
            | VarInit { $$ = new AST::VarInitList(); $$->push_back($1); }

VarInit : IDENTIFIER ASSIGN Expr { $$ = new AST::VarInit(*$1, $3); }
        | IDENTIFIER { $$ = new AST::VarInit(*$1); }

TypeSpecifier : BuiltInType { $$ = $1; }

BuiltInType : VOID { $$ = new AST::BuiltInType(AST::BuiltInType::_VOID); }
	    | BOOL { $$ = new AST::BuiltInType(AST::BuiltInType::_BOOL); }
            | CHAR { $$ = new AST::BuiltInType(AST::BuiltInType::_CHAR); }
            | INT { $$ = new AST::BuiltInType(AST::BuiltInType::_INT); }

Params : Params COMMA Param { $$ = $1; $$->push_back($3); }
       | Param { $$ = new AST::Params(); $$->push_back($1); }
       | VOID { $$ = new AST::Params(); }
       | { $$ = new AST::Params(); }

Param : TypeSpecifier IDENTIFIER { $$ = new AST::Param($1, *$2); }

Block : LBRACE Stmts RBRACE { $$ = new AST::Block($2); }

Stmts : Stmts Stmt { $$ = $1; $$->push_back($2); }
      | Stmt { $$ = new AST::Stmts(); $$->push_back($1); }

Stmt : VarDef { $$ = $1; }
     | Block { $$ = $1; }
     | Expr SEMI { $$ = new AST::ExprStmt($1); }
     | IfStmt { $$ = $1; }
     | ReturnStmt { $$ = $1; }

IfStmt : IF LPAREN Expr RPAREN Stmt ELSE Stmt { $$ = new AST::IfStmt($3, $5, $7); }
       | IF LPAREN Expr RPAREN Stmt { $$ = new AST::IfStmt($3, $5); }

ReturnStmt : RETURN Expr SEMI { $$ = new AST::ReturnStmt($2); }
           | RETURN SEMI { $$ = new AST::ReturnStmt(); }

Expr : FuncCall { $$ = $1; }
     | Expr ADD Expr { $$ = new AST::AddExpr($1, $3); }
     | Expr EQUAL Expr { $$ = new AST::EqExpr($1, $3); }
     | IDENTIFIER { $$ = new AST::Variable(*$1); }
     | Constant { $$ = $1; }

Constant : TRUE { $$ = new AST::Boolean(true); }
	 | FALSE { $$ = new AST::Boolean(false); }
         | CHARACTER { $$ = new AST::Character($1); }
         | INTEGER { $$ = new AST::Integer($1); }
         | STRING { $$ = new AST::ConstString(*$1); }

FuncCall : IDENTIFIER LPAREN Args RPAREN { $$ = new AST::FuncCall(*$1, $3); }

Args : Args COMMA Expr { $$ = $1; $$->push_back($3); }
     | Expr { $$ = new AST::Args(); $$->push_back($1); }
     | { $$ = new AST::Args(); }

%%