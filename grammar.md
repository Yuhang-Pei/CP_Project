# Grammar of C

## Context-Free Grammar

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

