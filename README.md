# CERIcompiler

A simple compiler.
From : Pascal-like imperative LL(k) langage
To : 64 bit 80x86 assembly langage (AT&T)

**Download the repository :**

> git clone git@framagit.org:jourlin/cericompiler.git

**Build the compiler and test it :**

> make test

**Have a look at the output :**

> gedit test.s

**Debug the executable :**

> ddd ./test

**Commit the new version :**

> git commit -a -m "What's new..."

**Send to your framagit :**

> git push -u origin master

**Get from your framagit :**

> git pull -u origin master

**This version Can handle :**

Program             := [DeclarationPart] [VarDeclarationPart] StatementPart

DeclarationPart     := "[" Identifier {"," Identifier} "]"

VarDeclarationPart  := "VAR" VarDeclaration {";" VarDeclaration} "."
VarDeclaration      := Ident {"," Ident} ":" Type

StatementPart       := Statement {";" Statement} "."

Statement           := AssignementStatement | IfStatement | WhileStatement | DoWhileStatement | ForStatement | BlockStatement | RepeatStatement | CaseStatement | Display

AssignementStatement := Identifier ":=" Expression

IfStatement         := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
WhileStatement      := "WHILE" Expression "DO" Statement
DoWhileStatement    := "DO" Statement "WHILE" Expression
ForStatement        := "FOR" AssignementStatement "TO" Expression "DO" Statement | "FOR" AssignementStatement "DOWNTO" Expression "DO" Statement
BlockStatement      := "BEGIN" Statement {";" Statement} "END"
RepeatStatement     := "REPEAT" Statement {";" Statement} "UNTIL" Expression
CaseStatement       := "CASE" Expression "OF" CaseListElement {";" CaseListElement} "END"

CaseListElement     := CaseLabelList ":" Statement | <empty>
CaseLabelList       := Constant {"," Constant}
Constant            := Number | CharConst

Display             := "DISPLAY" Expression

Expression          := SimpleExpression [RelationalOperator SimpleExpression]
SimpleExpression    := Term {AdditiveOperator Term}
Term                := Factor {MultiplicativeOperator Factor}
Factor              := Number | CharConst | StringConst | Identifier | "(" Expression ")"

Number              := Digit {Digit} ["." Digit {Digit}]
Identifier          := Letter {Letter | Digit}

AdditiveOperator       := "+" | "-" | "||"
MultiplicativeOperator := "*" | "/" | "%" | "&&"
RelationalOperator     := "==" | "!=" | "<" | ">" | "<=" | ">="

Digit  := "0" | ... | "9"
Letter := "a" | ... | "z" | "A" | ... | "Z"



Un test complet a tester est disponible dans le fichier test.p