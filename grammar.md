# Grammar of C

## Context-Free Grammar

### Version 1.5

```
Prog : Units

Units : Units Unit
      | Unit

Unit : Def

Def : FuncDef
    | VarDef

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN FuncBody

FuncBody : LBRACE Stmts RBRACE

VarDef : TypeSpecifier VarInitList SEMI

VarInitList : VarInitList COMMA VarInit
            | VarInit

VarInit : IDENTIFIER ASSIGN Expr
        | IDENTIFIER

TypeSpecifier : BuiltInType

BuiltInType : VOID
            | BOOL
            | CHAR
            | INT

Params : Params COMMA Param
       | Param
       | VOID
       |

Param : TypeSpecifier IDENTIFIER

Block : LBRACE Stmts RBRACE

Stmts : Stmts Stmt
      | Stmt

Stmt : VarDef
     | Block
     | ExprStmt [修改]
     | IfStmt
     | ForStmt [新增]
     | ReturnStmt
     | EmptyStmt [新增]

IfStmt : IF LPAREN Expr RPAREN Stmt ELSE Stmt
       | IF LPAREN Expr RPAREN Stmt

ReturnStmt : RETURN Expr SEMI
           | RETURN SEMI [新增]

ForStmt : FOR LPAREN ForInit ForCondition SEMI ForIncrement RPAREN Stmt [新增]

ForInit : ExprStmt [新增]
        | VarDef [新增]
        | EmptyStmt [新增]

ForCondition : Expr [新增]
             | [新增]

ForIncrement : Expr [新增]
             | [新增]

ReturnStmt : RETURN Expr SEMI
           | RETURN SEMI

EmptyStmt : SEMI [新增]

Expr : FuncCall
     | Expr ADD Expr
     | Expr MUL Expr [新增]
     | Expr EQUAL Expr
     | Expr NEQ Expr [新增]
     | Expr ASSIGN Expr [新增]
     | IDENTIFIER
     | Constant

Constant : TRUE
         | FALSE
  			 | CHARACTER
         | INTEGER
         | STRING [新增]

FuncCall : IDENTIFIER LPAREN Args RPAREN

Args : Args COMMA Expr
     | Expr
     |
```





### Version 1.4

```
Prog : Units

Units : Units Unit
      | Unit

Unit : Def

Def : FuncDef
    | VarDef

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN FuncBody

FuncBody : LBRACE Stmts RBRACE

VarDef : TypeSpecifier VarInitList SEMI

VarInitList : VarInitList COMMA VarInit
            | VarInit

VarInit : IDENTIFIER ASSIGN Expr
        | IDENTIFIER

TypeSpecifier : BuiltInType

BuiltInType : VOID
            | BOOL
            | CHAR
            | INT

Params : Params COMMA Param
       | Param
       | VOID
       |

Param : TypeSpecifier IDENTIFIER

Block : LBRACE Stmts RBRACE

Stmts : Stmts Stmt
      | Stmt

Stmt : VarDef
     | Block
     | Expr SEMI
     | IfStmt [新增]
     | ReturnStmt

IfStmt : IF LPAREN Expr RPAREN Stmt ELSE Stmt [新增]
       | IF LPAREN Expr RPAREN Stmt [新增]

ReturnStmt : RETURN Expr SEMI
           | RETURN SEMI [新增]

Expr : FuncCall
     | Expr ADD Expr
     | Expr EQUAL Expr [新增]
     | IDENTIFIER
     | Constant

Constant : TRUE
         | FALSE
  			 | CHARACTER
         | INTEGER

FuncCall : IDENTIFIER LPAREN Args RPAREN

Args : Args COMMA Expr
     | Expr
     |
```



### Version 1.3

```
Prog : Units

Units : Units Unit
      | Unit

Unit : Def

Def : FuncDef
    | VarDef

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN FuncBody [修改]

FuncBody : LBRACE Stmts RBRACE [新增]

VarDef : TypeSpecifier VarInitList SEMI

VarInitList : VarInitList COMMA VarInit
            | VarInit

VarInit : IDENTIFIER ASSIGN Expr
        | IDENTIFIER

TypeSpecifier : BuiltInType

BuiltInType : VOID
            | BOOL [新增]
            | CHAR
            | INT

Params : Params COMMA Param
       | Param
       | VOID
       |

Param : TypeSpecifier IDENTIFIER

Block : LBRACE Stmts RBRACE

Stmts : Stmts Stmt
      | Stmt

Stmt : VarDef [新增]
     | Block [新增]
     | Expr SEMI
     | ReturnStmt [修改]

ReturnStmt : RETURN Expr SEMI [新增]

Expr : FuncCall
     | Expr ADD Expr
     | IDENTIFIER
     | Constant

Constant : TRUE [新增]
         | FALSE [新增]
  			 | CHARACTER
         | INTEGER

FuncCall : IDENTIFIER LPAREN Args RPAREN

Args : Args COMMA Expr
     | Expr
     | 
```



### Version 1.2

```c
int a(int arg1, int arg2) {
    int u = 200, v = u;
    return v + 2 + arg2;
}

int main(void) {
    char c = 'c';
    printInt(1 + 10 + 100);
    printInt(a(2, 20));
    printChar('a');
    printChar(c);

    return 0;
}
```

```
Prog : Units

Units : Units Unit
      | Unit

Unit : Def

Def : FuncDef
    | VarDef

FuncDef : TypeSpecifier IDENTIFIER LPAREN Params RPAREN Block

VarDef : TypeSpecifier VarInitList SEMI

VarInitList : VarInitList COMMA VarInit
            | VarInit

VarInit : IDENTIFIER ASSIGN Expr
        | IDENTIFIER

TypeSpecifier : BuiltInType

BuiltInType : VOID
            | CHAR [新增]
            | INT

Params : Params COMMA Param
       | Param
       | VOID
       |

Param : TypeSpecifier IDENTIFIER

Block : LBRACE Stmts RBRACE

Stmts : VarDef
      | Stmts Stmt
      | Stmt

Stmt : Expr SEMI
     | RETURN Expr SEMI

Expr : FuncCall
     | Expr ADD Expr
     | IDENTIFIER
     | Constant [新增]

Constant : CHARACTER [新增]
         | INTEGER [新增]

FuncCall : IDENTIFIER LPAREN Args RPAREN

Args : Args COMMA Expr
     | Expr
     | [新增]
```



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

