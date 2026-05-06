//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <map>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>

using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};

TOKEN current;				// Current token


FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

	
map<string, enum TYPES> DeclaredVariables;
unsigned long long TagNumber=0;

bool IsDeclared(const char *id){
	return DeclaredVariables.find(id)!=DeclaredVariables.end();
}


void Error(string s){
	cerr << "Ligne n°"<<lexer->lineno()<<", lu : '"<<lexer->YYText()<<"'("<<current<<"), mais ";
	cerr<< s << endl;
	exit(-1);
}

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"
	
		
enum TYPES Identifier(void){
	enum TYPES type;
	type=DeclaredVariables[lexer->YYText()];
	cout << "\tpush "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();
	return type;
}

// Number := Digit{Digit}
enum TYPES Number(void){
	bool decimal = false;
	double d;
	unsigned int *i;
	string nombre = lexer->YYText();
	if(nombre.find(".")!=string::npos){
		// Floating point constant number
		d=atof(lexer->YYText());
		i=(unsigned int *) &d; // i points to the const double
		//cout <<"\tpush $"<<*i<<"\t# Conversion of "<<d<<endl;
		// Is equivalent to : 
		cout <<"\tsubq $8,%rsp\t\t\t# allocate 8 bytes on stack's top"<<endl;
		cout <<"\tmovl	$"<<*i<<", (%rsp)\t# Conversion of "<<d<<" (32 bit high part)"<<endl;
		cout <<"\tmovl	$"<<*(i+1)<<", 4(%rsp)\t# Conversion of "<<d<<" (32 bit low part)"<<endl;
		current=(TOKEN) lexer->yylex();
		return DOUBLE;
	}
	else {
		cout <<"\tpush $"<<atoi(lexer->YYText())<<endl;
		current=(TOKEN) lexer->yylex();
		return INTEGER;
	}
}

enum TYPES CharConst(void){
    string s = lexer->YYText();
    char c = s[1];  // extraire le caractère entre les apostrophes
    
    // Gérer les séquences d'échappement
    if(c == '\\' && s.length() >= 3){
        switch(s[2]){
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case '\\': c = '\\'; break;
            case '\'': c = '\''; break;
            default: c = s[2]; break;
        }
    }
    
    cout << "\tmovq $0, %rax" << endl;
    cout << "\tmovb $" << (int)c << ", %al\t# char '" << s << "'" << endl;
    cout << "\tpush %rax" << endl;
    current = (TOKEN) lexer->yylex();
    return CHAR;
}

enum TYPES Expression(void);			// Called by Term() and calls Term()

// Factor := Number | Letter | "(" Expression ")"| "!" Factor
enum TYPES Factor(void){
	enum TYPES type;
	switch(current){
		case RPARENT :
			current=(TOKEN) lexer->yylex();
			type=Expression();
			if(current!=LPARENT)
				Error("')' était attendu");		// ")" expected
			else
				current=(TOKEN) lexer->yylex();
			break;
		case NUMBER : 
			type = Number();
			break;
		case ID: 
			type = Identifier();
			break;
		case CHARCONST :
			type = CharConst();
			break;
		default : 
			Error("'(', ou constante ou variable attendue.");
	};
	return type;
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator(void){
	OPMUL opmul;
	if(strcmp(lexer->YYText(),"*")==0)
		opmul=MUL;
	else if(strcmp(lexer->YYText(),"/")==0)
		opmul=DIV;
	else if(strcmp(lexer->YYText(),"%")==0)
		opmul=MOD;
	else if(strcmp(lexer->YYText(),"&&")==0)
		opmul=AND;
	else opmul=WTFM;
	current=(TOKEN) lexer->yylex();
	return opmul;
}

// Term := Factor {MultiplicativeOperator Factor}
enum TYPES Term(void){
	OPMUL mulop;
	enum TYPES type1, type2;
	type1=Factor();
	while(current==MULOP){
		mulop=MultiplicativeOperator();		// Save operator in local variable
		type2=Factor();
		if(type1!=type2){
			Error("Type differents/incompatibles pour Term");
		}
		switch(mulop){
			case AND:
				if(type2!=BOOLEAN)
					Error("type booléen requis pour l'opérateur AND");
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# AND"<<endl;	// store result
				break;
			case MUL:
				if(type2!=INTEGER&&type2!=DOUBLE)
					Error("type numérique requis pour la multiplication");
				if(type2==INTEGER){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
					cout << "\tpush %rax\t# MUL"<<endl;	// store result
				}else{
					cout<<"\tfldl	8(%rsp)\t"<<endl;
					cout<<"\tfldl	(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfmulp	%st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;
			case DIV:
				if(type2!=INTEGER&&type2!=DOUBLE)
					Error("type numérique requis pour la division");
				if(type2==INTEGER){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tmovq $0, %rdx"<<endl; 	
					cout << "\tdiv %rbx"<<endl;			
					cout << "\tpush %rax\t# DIV"<<endl;		// store result
				}
				else{
					cout<<"\tfldl	(%rsp)\t"<<endl;
					cout<<"\tfldl	8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfdivp	%st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;
			case MOD:
				if(type2!=INTEGER)
					Error("type non entier pour le modulo");
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
				cout << "\tpush %rdx\t# MOD"<<endl;		// store result
				break;

			default:
				Error("opérateur multiplicatif attendu");
		}
	}
	return type1;
}

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator(void){
	OPADD opadd;
	if(strcmp(lexer->YYText(),"+")==0)
		opadd=ADD;
	else if(strcmp(lexer->YYText(),"-")==0)
		opadd=SUB;
	else if(strcmp(lexer->YYText(),"||")==0)
		opadd=OR;
	else opadd=WTFA;
	current=(TOKEN) lexer->yylex();
	return opadd;
}

// SimpleExpression := Term {AdditiveOperator Term}
enum TYPES SimpleExpression(void){
	enum TYPES type1, type2;
	OPADD adop;
	type1 = Term();
	while(current==ADDOP){
		adop=AdditiveOperator();		// Save operator in local variable
		type2 = Term();
		if(type1!=type2){
			Error("Type differents/incompatibles pour SimpleExpression");
		}
		switch(adop){
			case OR:
				if(type2!=BOOLEAN)
					Error("opérande booléenne requis pour l'opérateur OR");
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
				cout << "\taddq	%rbx, %rax\t# OR"<<endl;// operand1 OR operand2
				cout << "\tpush %rax" << endl;
				break;			
			case ADD:
				if(type2!=INTEGER&&type2!=DOUBLE)
					Error("opérande numérique requis pour l'addition");
				if(type2==INTEGER){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
					cout << "\tpush %rax"<<endl;			// store result
				}
				else{
					cout<<"\tfldl	8(%rsp)\t"<<endl;
					cout<<"\tfldl	(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfaddp	%st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;				
			case SUB:	
				if(type2!=INTEGER&&type2!=DOUBLE)
					Error("opérande numérique requis pour la soustraction");
				if(type2==INTEGER){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tsubq	%rbx, %rax\t# ADD"<<endl;	// add both operands
					cout << "\tpush %rax"<<endl;			// store result
				}
				else{
					cout<<"\tfldl	(%rsp)\t"<<endl;
					cout<<"\tfldl	8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfsubp	%st(0),%st(1)\t# %st(0) <- op1 - op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;	
			default:
				Error("opérateur additif inconnu");
		}
	}
	return type1;
}

// DeclarationPart := "[" Ident {"," Ident} "]"
void DeclarationPart(void){
	if(current!=RBRACKET)
		Error("caractère '[' attendu");
	cout << "\t.data"<<endl;
	cout << "\t.align 8"<<endl;
	
	current=(TOKEN) lexer->yylex();
	if(current!=ID)
		Error("Un identificater était attendu");
	cout << lexer->YYText() << ":\t.quad 0"<<endl;
	DeclaredVariables[lexer->YYText()];
	current=(TOKEN) lexer->yylex();
	while(current==COMMA){
		current=(TOKEN) lexer->yylex();
		if(current!=ID)
			Error("Un identificateur était attendu");
		cout << lexer->YYText() << ":\t.quad 0"<<endl;
		DeclaredVariables[lexer->YYText()];
		current=(TOKEN) lexer->yylex();
	}
	if(current!=LBRACKET)
		Error("caractère ']' attendu");
	current=(TOKEN) lexer->yylex();
}

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void){
	OPREL oprel;
	if(strcmp(lexer->YYText(),"==")==0)
		oprel=EQU;
	else if(strcmp(lexer->YYText(),"!=")==0)
		oprel=DIFF;
	else if(strcmp(lexer->YYText(),"<")==0)
		oprel=INF;
	else if(strcmp(lexer->YYText(),">")==0)
		oprel=SUP;
	else if(strcmp(lexer->YYText(),"<=")==0)
		oprel=INFE;
	else if(strcmp(lexer->YYText(),">=")==0)
		oprel=SUPE;
	else oprel=WTFR;
	current=(TOKEN) lexer->yylex();
	return oprel;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
enum TYPES Expression(void){
	enum TYPES type1, type2;
	unsigned long long tag;
	OPREL oprel;
	type1=SimpleExpression();
	if(current==RELOP){
		tag=++TagNumber;
		oprel=RelationalOperator();
		type2=SimpleExpression();
		if(type1!=type2){
			Error("Type differents/incompatibles pour Expression");
		}
		if(type1!=DOUBLE){
			cout << "\tpop %rax"<<endl;
			cout << "\tpop %rbx"<<endl;
			cout << "\tcmpq %rax, %rbx"<<endl;
		}
		else{
			cout<<"\tfldl	(%rsp)\t"<<endl;
			cout<<"\tfldl	8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
			cout<<"\t addq $16, %rsp\t# 2x pop nothing"<<endl;
			cout<<"\tfcomip %st(1)\t\t# compare op1 and op2 -> %RFLAGS and pop"<<endl;
			cout<<"\tfaddp %st(1)\t# pop nothing"<<endl;
		}
		switch(oprel){
			case EQU:
				cout << "\tje Vrai"<<tag<<"\t# If equal"<<endl;
				break;
			case DIFF:
				cout << "\tjne Vrai"<<tag<<"\t# If different"<<endl;
				break;
			case SUPE:
				cout << "\tjae Vrai"<<tag<<"\t# If above or equal"<<endl;
				break;
			case INFE:
				cout << "\tjbe Vrai"<<tag<<"\t# If below or equal"<<endl;
				break;
			case INF:
				cout << "\tjb Vrai"<<tag<<"\t# If below"<<endl;
				break;
			case SUP:
				cout << "\tja Vrai"<<tag<<"\t# If above"<<endl;
				break;
			default:
				Error("Opérateur de comparaison inconnu");

		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<tag<<endl;
		cout << "Vrai"<<tag<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout << "Suite"<<tag<<":"<<endl;
		return BOOLEAN;
	}
	return type1;
}

// AssignementStatement := Identifier ":=" Expression
enum TYPES AssignementStatement(void){
	enum TYPES type1, type2;
	string variable;
	if(current!=ID)
		Error("Identificateur attendu");
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	type1=DeclaredVariables[lexer->YYText()];	
	variable=lexer->YYText();
	current=(TOKEN) lexer->yylex();
	if(current!=ASSIGN)
		Error("caractères ':=' attendus");
	current=(TOKEN) lexer->yylex();
	type2=Expression();
	if (type1!=type2){
		Error("Type differents/incompatibles pour AssignementStatement");
	}
	switch(type1){
        case DOUBLE:
            cout << "\tmovsd (%rsp), %xmm0" << endl;
            cout << "\tmovsd %xmm0, " << variable << "(%rip)" << endl;
            cout << "\taddq $8, %rsp" << endl;
            break;
        default:
            cout << "\tpop " << variable << endl;
            break;
    }
	return type1;
}

void Statement(void);


//IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement (void){
	enum TYPES type;
	if (current== IF){
		current=(TOKEN) lexer->yylex();
		type=Expression();
		if (type != BOOLEAN){
			Error("type non boolean dans le IF");
		}
		if (current == THEN){
			current=(TOKEN) lexer->yylex();
			Statement();
			if (current== ELSE){
				current=(TOKEN) lexer->yylex();
				Statement();
			}
		}
		else {
			Error("Texte 'THEN' attendu");
		}
	}
	else {
			Error("Texte 'IF' attendu");
		}
}


//WhileStatement := "WHILE" Expression "DO" Statement
void WhileStatement(void){
	enum TYPES type;
	if (current== WHILE){
		current=(TOKEN) lexer->yylex();
		type=Expression();
		if (type != BOOLEAN){
			Error("type non boolean dans le While");
		}
		if (current== DO){
			current=(TOKEN) lexer->yylex();
			Statement();
		}
		else {
			Error("Texte 'DO' attendu");
		}
	}
	else {
			Error("Texte 'WHILE' attendu");
		}
}


//ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
void ForStatement(void){
	if (current== FOR){
		current=(TOKEN) lexer->yylex();
		AssignementStatement();
		if (current== TO){
			current=(TOKEN) lexer->yylex();
			Expression();
			if (current == DO){
				current=(TOKEN) lexer->yylex();
				Statement();
			}
			else {
				Error("Texte 'DO' attendu");
			}
		}
		else {
				Error("Texte 'TO' attendu");
			}
	}
	else {
				Error("Texte 'FOR' attendu");
			}
}

//BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement(void) {
	if (current == BEG){
		current=(TOKEN) lexer->yylex();
		Statement();
		while (current == SEMICOLON) {
			current=(TOKEN) lexer->yylex();
			Statement();
		}
		if (current!= END) {
			Error("texte 'END' Attendu");
		}
		current=(TOKEN) lexer->yylex();
	}
	else {
		Error("Texte 'BEGIN' Attendu");
	}
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==SEMICOLON){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=DOT)
		Error("caractère '.' attendu");
	current=(TOKEN) lexer->yylex();
}

// DISPLAY := "DISPLAY" <expression>
void Display(void){
	unsigned long long tag=++TagNumber;
	enum TYPES type;
	if (current==DISPLAY){
		current=(TOKEN) lexer->yylex();
		type=Expression();
		switch(type){
			case INTEGER : 
				cout << "\tpop %rsi\t# The value to be displayed"<<endl;
				cout << "\tmovq $FormatString1, %rdi\t# \"%llu\\n\""<<endl;
				cout << "\tmovl	$0, %eax"<<endl;
				cout << "\tcall	printf@PLT"<<endl;
				break;
			case BOOLEAN : 
				cout << "\tpop %rdx\t# Zero : False, non-zero : true"<<endl;
				cout << "\tcmpq $0, %rdx"<<endl;
				cout << "\tje False"<<tag<<endl;
				cout << "\tmovq $TrueString, %rdi\t# \"TRUE\\n\""<<endl;
				cout << "\tjmp Next"<<tag<<endl;
				cout << "False"<<tag<<":"<<endl;
				cout << "\tmovq $FalseString, %rdi\t# \"FALSE\\n\""<<endl;
				cout << "Next"<<tag<<":"<<endl;
				cout << "\tcall	puts@PLT"<<endl;
				break;
			case CHAR : 
				cout<<"\tpop %rsi\t\t\t# get character in the 8 lowest bits of %si"<<endl;
				cout << "\tmovq $FormatString3, %rdi\t# \"%c\\n\""<<endl;
				cout << "\tmovl	$0, %eax"<<endl;
				cout << "\tcall	printf@PLT"<<endl;
				break;
			case DOUBLE : 
    			cout << "\tmovsd (%rsp), %xmm0" << endl;
    			cout << "\taddq $8, %rsp\t\t# Pop la valeur" << endl;
    			// Alignement de la pile (si nécessaire) et appel
   			 	cout << "\tmovl $1, %eax\t\t# 1 argument float" << endl;
   				cout << "\tmovq $FormatString2, %rdi" << endl;
    			cout << "\tandq $-16, %rsp\t\t# Alignement 16-bytes" << endl;
    			cout << "\tcall printf@PLT" << endl;
				break;
			default: 
				Error("DISPLAY ne fontionne que pour les entier, boolean, caractere et double");
		}
	}
	else{
		Error("Mot clé DISPLAY attendu");
	}
}

enum TYPES check_type (void){
	if(strcmp(lexer->YYText(),"BOOLEAN")==0){
		return BOOLEAN;
	}	
	else if(strcmp(lexer->YYText(),"INTEGER")==0){
		return INTEGER;
	}
	else if(strcmp(lexer->YYText(),"DOUBLE")==0){
		return DOUBLE;
	}
	else if(strcmp(lexer->YYText(),"CHAR")==0){
		return CHAR;
	}
	else {
		Error ("Erreur type attendu");
	}
}

// VarDeclaration := Ident {"," Ident} ":" Type
void VarDeclaration(void){
    enum TYPES type;
    set<string> idents;   

    if (current == ID) {
        idents.insert(lexer->YYText());          
        current = (TOKEN) lexer->yylex();

        while (current == COMMA) {
            current = (TOKEN) lexer->yylex();
            if (current == ID) {
                idents.insert(lexer->YYText());  
                current = (TOKEN) lexer->yylex();
            } else {
                Error("Erreur Ident requis");
            }
        }

        if (current == COLON) {
            current = (TOKEN) lexer->yylex();
            type = check_type();                 
            current = (TOKEN) lexer->yylex();

            for (set<string>::iterator it = idents.begin(); it != idents.end(); ++it) {
                switch (type) {
                    case BOOLEAN:
                    case INTEGER:
                        cout << *it << ":\t.quad 0" << endl;
                        break;
                    case DOUBLE:
                        cout << *it << ":\t.double 0.0" << endl;  
                        break;
                    case CHAR:
                        cout << *it << ":\t.byte 0" << endl;      
                        break;
                    default:
                        Error("type inconnu.");
                }
                DeclaredVariables[*it] = type;   
            }
        } else {
            Error("Erreur signe ':' attendu");
        }
    } else {
        Error("Erreur Ident requis");
    }
}

// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
void VarDeclarationPart(void){
	if (current==VAR){
		current=(TOKEN) lexer->yylex();
		VarDeclaration();
		while (current==SEMICOLON){
			current=(TOKEN) lexer->yylex();
			VarDeclaration();	
		}
		if (current!=DOT){
			Error("le signe '.' est attendu ");
		}
		current=(TOKEN) lexer->yylex();
	}
	else {
		Error("le texte 'VAR' est attendu ");
	}
}


//Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | Display
void Statement(void){
	if(current==ID){
		AssignementStatement();
	}
	else if(current==IF){
		IfStatement();
	}
	else if(current==WHILE){
		WhileStatement();
	}
	else if(current==FOR){
		ForStatement();
	}
	else if(current==BEG){
		BlockStatement();
	}
	else if(current==DISPLAY){
		Display();
	}
	else {
		Error("Erreur aucun mot clé renseigné");
	}
}

// Program := [DeclarationPart] StatementPart
void Program(void){
	if(current==RBRACKET)
		DeclarationPart();
	if(current==VAR)          
        VarDeclarationPart();
	StatementPart();	
}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	cout << ".data"<<endl;
	cout << "FormatString1:\t.string \"%llu\"\t# used by printf to display 64-bit unsigned integers"<<endl; 
	cout << "FormatString2:\t.string \"%g\"\t# used by printf to display 64-bit floating point numbers"<<endl; 
	cout << "FormatString3:\t.string \"%c\"\t# used by printf to display a 8-bit single character"<<endl; 
	cout << "TrueString:\t.string \"TRUE\"\t# used by printf to display the boolean value TRUE"<<endl; 
	cout << "FalseString:\t.string \"FALSE\"\t# used by printf to display the boolean value FALSE"<<endl; 
	// Let's proceed to the analysis and code production
	current=(TOKEN) lexer->yylex();
	Program();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(current!=FEOF){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}