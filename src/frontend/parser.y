%{

#include <cstdio>
#include <cstdlib>

#include "AST.h"
#include "codegen.h"

extern int yylex(void);
void yyerror(const char *str) {
    std::cout << "Error: " << str << std::endl;
    exit(1);
}

AST::Prog *Root;

AST::TypeSpecifier *CurrentBaseType;

std::string *CurrentVarName;

%}

%union {
    char charVal;
    int intVal;
    double doubleVal;
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
    AST::ArrType *arrType;
    AST::PtrType *ptrType;

    AST::Stmt *stmt;
    AST::Stmts *stmts;
    AST::Block *block;
    AST::ExprStmt *exprStmt;
    AST::IfStmt *ifStmt;
    AST::ForStmt *forStmt;
    AST::ReturnStmt *returnStmt;
    AST::EmptyStmt * emptyStmt;

    AST::Expr *expr;
    AST::FuncCall *funcCall;
    AST::Args *args;
    AST::AddExpr *addExpr;
    AST::MulExpr *mulExpr;
    AST::SubExpr *subExpr;
    AST::DivExpr *divExpr;
    AST::EqExpr *eqExpr;
    AST::NeqExpr *neqExpr;
    AST::GreatExpr *greatExpr;
    AST::LessExpr *lessExpr;
    AST::AssignExpr *assignExpr;
    AST::Variable *variable;
    AST::Constant *constant;
    AST::Boolean *boolean;
    AST::Character *character;
    AST::Integer *integer;
    AST::Real *real;
}

%token<token>		TRUE FALSE NULLPTR
%token<charVal>		CHARACTER
%token<intVal>		INTEGER
%token<doubleVal>   	REAL
%token<strVal>		STRING
%token<identifier>	IDENTIFIER
%token<token>		SEMI COMMA DOT LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token<token>		ADD SUB MUL DIV
%token<token>		EQUAL NEQ
%token<token>		GREAT LESS
%token<token>		NOT
%token<token>		ASSIGN
%token<token>		VOID BOOL CHAR INT DOUBLE
%token<token>		IF ELSE FOR RETURN

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

%type<typeSpecifier>	TypeSpecifier VarDefBaseType ComplexVar
%type<builtInType>	BuiltInType
%type<intVal>		ArrSize

%type<stmt>		Stmt
%type<stmts>		Stmts
%type<block>		Block
%type<exprStmt>		ExprStmt
%type<ifStmt>		IfStmt
%type<forStmt>		ForStmt
%type<stmt>		ForInit
%type<expr>		ForCondition ForIncrement
%type<returnStmt>	ReturnStmt
%type<emptyStmt>	EmptyStmt

%type<expr>		Expr
%type<funcCall>		FuncCall
%type<args>		Args
%type<constant>		Constant

%type<identifier>	IdentifierUse

%left   ADD SUB
%left   MUL DIV
%right	NOT
%left 	DOT

%start  Prog

%%

Prog : Units { $$ = new AST::Prog($1); Root = $$; }

Units : Units Unit { $$ = $1; $$->push_back($2); }
      | Unit { $$ = new AST::Units(); $$->push_back($1); }

Unit : Def { $$ = $1; }

Def : FuncDef { $$ = $1; }
    | VarDef { $$ = $1; }

FuncDef : TypeSpecifier IdentifierUse LPAREN Params RPAREN FuncBody { $$ = new AST::FuncDef($1, *$2, $4, $6); }

FuncBody : LBRACE Stmts RBRACE { $$ = new AST::FuncBody($2); }

VarDef : VarDefBaseType VarInitList SEMI { $$ = new AST::VarDef($1, $2); }

VarDefBaseType : TypeSpecifier { $$ = $1; CurrentBaseType = $$; }

VarInitList : VarInitList COMMA VarInit { $$ = $1; $$->push_back($3); }
            | VarInit { $$ = new AST::VarInitList(); $$->push_back($1); }

VarInit : IdentifierUse ASSIGN Expr { $$ = new AST::VarInit(*$1, $3); }
        | IdentifierUse { $$ = new AST::VarInit(*$1); }
	| ComplexVar { $$ = new AST::VarInit(*CurrentVarName, $1, CurrentBaseType); }

ComplexVar : IdentifierUse ArrSize %prec DOT { CurrentVarName = $1; $$ = new AST::ArrType(nullptr, $2); }
	   | MUL IdentifierUse %prec NOT { CurrentVarName = $2; $$ = new AST::PtrType(nullptr); }
	   | ComplexVar ArrSize %prec DOT { $$ = new AST::ArrType($1, $2); }
	   | MUL ComplexVar %prec NOT { $$ = new AST::PtrType($2); }
	   | LPAREN ComplexVar RPAREN %prec DOT { $$ = $2; }

ArrSize : LBRACKET TRUE RBRACKET { $$ = true; }
	| LBRACKET FALSE RBRACKET { $$ = false; }
	| LBRACKET CHARACTER RBRACKET { $$ = $2; }
	| LBRACKET INTEGER RBRACKET { $$ = $2; }

TypeSpecifier : BuiltInType { $$ = $1; }

BuiltInType : VOID { $$ = new AST::BuiltInType(AST::BuiltInType::_VOID); }
	    | BOOL { $$ = new AST::BuiltInType(AST::BuiltInType::_BOOL); }
            | CHAR { $$ = new AST::BuiltInType(AST::BuiltInType::_CHAR); }
            | INT { $$ = new AST::BuiltInType(AST::BuiltInType::_INT); }
            | DOUBLE {$$ = new AST::BuiltInType(AST::BuiltInType::_DOUBLE); }

Params : Params COMMA Param { $$ = $1; $$->push_back($3); }
       | Param { $$ = new AST::Params(); $$->push_back($1); }
       | VOID { $$ = new AST::Params(); }
       | { $$ = new AST::Params(); }

Param : TypeSpecifier IdentifierUse { $$ = new AST::Param($1, *$2); }

Block : LBRACE Stmts RBRACE { $$ = new AST::Block($2); }

Stmts : Stmts Stmt { $$ = $1; $$->push_back($2); }
      | Stmt { $$ = new AST::Stmts(); $$->push_back($1); }

Stmt : VarDef { $$ = $1; }
     | Block { $$ = $1; }
     | ExprStmt { $$ = $1; }
     | IfStmt { $$ = $1; }
     | ForStmt { $$ = $1; }
     | ReturnStmt { $$ = $1; }
     | EmptyStmt { $$ = $1; }

ExprStmt : Expr SEMI { $$ = new AST::ExprStmt($1); }

IfStmt : IF LPAREN Expr RPAREN Stmt ELSE Stmt { $$ = new AST::IfStmt($3, $5, $7); }
       | IF LPAREN Expr RPAREN Stmt { $$ = new AST::IfStmt($3, $5); }

ForStmt : FOR LPAREN ForInit ForCondition SEMI ForIncrement RPAREN Stmt { $$ = new AST::ForStmt($3, $4, $6, $8); }

ForInit : ExprStmt { $$ = $1; }
        | VarDef { $$ = $1; }
        | EmptyStmt { $$ = $1; }

ForCondition : Expr { $$ = $1; }
             | { $$ = nullptr; }

ForIncrement : Expr { $$ = $1; }
             | { $$ = nullptr; }

ReturnStmt : RETURN Expr SEMI { $$ = new AST::ReturnStmt($2); }
           | RETURN SEMI { $$ = new AST::ReturnStmt(); }

EmptyStmt : SEMI { $$ = new AST::EmptyStmt(); }

Expr : FuncCall { $$ = $1; }
     | Expr ADD Expr { $$ = new AST::AddExpr($1, $3); }
     | Expr MUL Expr { $$ = new AST::MulExpr($1, $3); }
     | Expr SUB Expr { $$ = new AST::SubExpr($1, $3); }
     | Expr DIV Expr { $$ = new AST::DivExpr($1, $3); }
     | Expr EQUAL Expr { $$ = new AST::EqExpr($1, $3); }
     | Expr NEQ Expr { $$ = new AST::NeqExpr($1, $3); }
     | Expr GREAT Expr { $$ = new AST::GreatExpr($1, $3); }
     | Expr LESS Expr { $$ = new AST::LessExpr($1, $3); }
     | Expr ASSIGN Expr { $$ = new AST::AssignExpr($1, $3); }
     | IdentifierUse { $$ = new AST::Variable(*$1); }
     | Constant { $$ = $1; }

Constant : TRUE { $$ = new AST::Boolean(true); }
	 | FALSE { $$ = new AST::Boolean(false); }
         | CHARACTER { $$ = new AST::Character($1); }
         | INTEGER { $$ = new AST::Integer($1); }
         | REAL { $$ = new AST::Real($1); }
         | STRING { $$ = new AST::ConstString(*$1); }

FuncCall : IdentifierUse LPAREN Args RPAREN { $$ = new AST::FuncCall(*$1, $3); }

Args : Args COMMA Expr { $$ = $1; $$->push_back($3); }
     | Expr { $$ = new AST::Args(); $$->push_back($1); }
     | { $$ = new AST::Args(); }

IdentifierUse : LPAREN IdentifierUse RPAREN { $$ = $2; }
	      | IDENTIFIER { $$ = $1; }

%%