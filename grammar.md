# Grammar of C

## Context-Free Grammar

### Version 1.1

```
Prog : Units

Units : Units Unit
      | Unit

Unit : Def

Def : FuncDef
    | VarDef [新增]

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN Block

VarDef : TypeSpecifier VarInitList SEMI [新增]

VarInitList : VarInitList COMMA VarInit [新增]
            | VarInit [新增]

VarInit : IDENTIFIER ASSIGN Expr [新增]
        | IDENTIFIER [新增]

TypeSpecifier : BuiltInType

BuiltInType : VOID
            | INT

Params : Params COMMA Param [新增]
       | Param [新增]
       | VOID
       | [新增]

Param : TypeSpecifier IDENTIFIER [新增]

Block : LBRACE Stmts RBRACE

Stmts : VarDef [新增]
      | Stmts Stmt
      | Stmt

Stmt : Expr SEMI
     | RETURN Expr SEMI [新增]

Expr : FuncCall
     | Expr ADD Expr
     | IDENTIFIER [新增]
     | INTEGER

FuncCall : IDENTIFIER LPAREN Args RPAREN

Args : Args COMMA Expr
     | Expr
     | [新增]
```



### Version 1.0		

```c
void main(void) {
    print(1 + 10 + 100);
}
```

```
Prog : Units

Units : Units Unit
      | Unit

Unit : Def

Def : FuncDef

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN Block

TypeSpecifier : BuiltInType

BuiltInType : VOID
            | INT

Params : VOID

Block : LBRACE Stmts RBRACE

Stmts : Stmts Stmt
      | Stmt

Stmt : Expr SEMI

Expr : FuncCall
     | Expr ADD Expr
     | INTEGER

FuncCall : IDENTIFIER LPAREN Args RPAREN

Args : Args COMMA Expr
     | Expr
```

