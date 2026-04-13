//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly language
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

using namespace std;

char current = EOF;
char nextcar;

void ReadChar(void){
    if(current == EOF)
        cin.get(nextcar);
    do {
        current = nextcar;
        cin.get(nextcar);
    } while(current != EOF && (current==' ' || current=='\t' || current=='\n'));
}

void Error(string s){
    cerr << s << endl;
    exit(-1);
}



void AdditiveOperator(char &op){
    if(current=='+' || current=='-'){
        op = current;
        ReadChar();
    }
    else if(current=='|' && nextcar=='|'){
        op = '|';
        ReadChar();
        ReadChar();
    }
    else{
        Error("Opérateur additif attendu");
    }
}

void MultiplicativeOperator(char &op){
    if(current=='*' || current=='/' || current=='%'){
        op = current;
        ReadChar();
    }
    else if(current=='&' && nextcar=='&'){
        op = '&';
        ReadChar();
        ReadChar();
    }
    else{
        Error("Opérateur multiplicatif attendu");
    }
}

void Expression(void);

// Factor := Number | Letter | "(" Expression ")" | "!" Factor
void Factor(void){
    if(current>='0' && current<='9'){
        long long value = 0;
        while(current>='0' && current<='9'){
            value = value * 10 + (current - '0');
            ReadChar();
        }
        cout << "\tpushq $" << value << endl;
    }
    else if(current>='a' && current<='z'){
        cout << "\tpushq " << current << "(%rip)" << endl;
        ReadChar();
    }
    else if(current == '('){
        ReadChar();
        Expression();
        if(current != ')')
            Error("Caractère ')' attendu");
        ReadChar();
    }
    else if(current == '!'){
        ReadChar();                     
        Factor();
        cout << "\tpopq %rax" << endl;
        cout << "\tnotq %rax" << endl;
        cout << "\tpushq %rax" << endl;
    }
    else{
        Error("Facteur attendu");
    }
}

// Term := Factor {MultiplicativeOperator Factor}
void Term(void){
    Factor();
    while(current=='*' || current=='/' || current=='%' ||
          (current=='&' && nextcar=='&')){
        char op;
        MultiplicativeOperator(op);
        Factor();

        cout << "\tpopq %rbx" << endl;
        cout << "\tpopq %rax" << endl;
        if(op=='*')      cout << "\taddq %rbx, %rax" << endl;
        else if(op=='/') cout << "\tsubq %rbx, %rax" << endl;
        else if(op=='%') cout << "\torq  %rbx, %rax" << endl;
        cout << "\tpushq %rax" << endl;
    }
}

// SimpleExpression := Term {AdditiveOperator Term}
void SimpleExpression(void){
    Term();
    while(current=='+' || current=='-' ||
          (current=='|' && nextcar=='|')){
        char op = current;
        AdditiveOperator(op);
        Term();

        cout << "\tpopq %rbx" << endl;
        cout << "\tpopq %rax" << endl;
        if(op=='+')      cout << "\taddq %rbx, %rax" << endl;
        else if(op=='-') cout << "\tsubq %rbx, %rax" << endl;
        else if(op=='|') cout << "\torq  %rbx, %rax" << endl;
        cout << "\tpushq %rax" << endl;
    }
}

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="
string RelationalOperator(void){
    string op = "";
    if(current=='<'){
        if(nextcar=='='){  op="<="; ReadChar(); ReadChar(); }
        else if(nextcar=='>'){ op="<>"; ReadChar(); ReadChar(); }
        else { op="<"; ReadChar(); }
    }
    else if(current=='>'){
        if(nextcar=='='){ op=">="; ReadChar(); ReadChar(); }
        else { op=">"; ReadChar(); }
    }
    else if(current=='!' && nextcar=='='){
        op="!="; ReadChar(); ReadChar();
    }
    else if(current=='=' && nextcar=='='){
        op="=="; ReadChar(); ReadChar();
    }
    else{
        Error("Opérateur relationnel attendu");
    }
    return op;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
void Expression(void){
    SimpleExpression();
    if(current=='<' || current=='>' ||
       (current=='!' && nextcar=='=') ||
       (current=='=' && nextcar=='=')){
        string op = RelationalOperator();
        SimpleExpression();
        cout << "\tpopq %rbx" << endl;
        cout << "\tpopq %rax" << endl;
        cout << "\tcmpq %rbx, %rax" << endl;
        cout << "\tmovq $0, %rax" << endl;
        if      (op=="==") cout << "\tsete  %al" << endl;
        else if (op=="!=") cout << "\tsetne %al" << endl;
        else if (op=="<")  cout << "\tsetl  %al" << endl;
        else if (op==">")  cout << "\tsetg  %al" << endl;
        else if (op=="<=") cout << "\tsetle %al" << endl;
        else if (op==">=") cout << "\tsetge %al" << endl;
        cout << "\tpushq %rax" << endl;
    }
}

// AssignmentStatement := Letter "=" Expression
void Assignment(void){
    if(!(current>='a' && current<='z'))
        Error("Lettre attendue");
    char varName = current;
    ReadChar();                          

    if(current != '=')
        Error("Caractère '=' attendu");
    ReadChar();                          

    Expression();

    cout << "\tpopq %rax" << endl;
    cout << "\tmovq %rax, " << varName << "(%rip)" << endl;
}

// Statement := AssignmentStatement
void Statement(void){
    Assignment();
}

// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
    Statement();
    while(current==';'){
        ReadChar();                      
        Statement();
    }
    if(current != '.')
        Error("Caractère '.' attendu");
}

// DeclarationPart := "[" Letter {"," Letter} "]"
void DeclarationPart(void){
    if(current != '[')
        Error("Caractère '[' attendu");
    ReadChar();                          

    if(!(current>='a' && current<='z'))
        Error("Lettre attendue");

    while(current>='a' && current<='z'){
        char varName = current;
        cout << "\t.comm " << varName << ", 8, 8" << endl;
        ReadChar();                      
        if(current==',')
            ReadChar();                  
    }

    if(current != ']')
        Error("Caractère ']' attendu");
    ReadChar();                          
}

// Program := [DeclarationPart] StatementPart
void Program(void){
    cout << "\t.bss" << endl;
    while(current == '['){
        DeclarationPart();
    }
    cout << "\t.text" << endl;
    cout << "\t.globl main" << endl;
    cout << "main:" << endl;
    StatementPart();
}

int main(void){
    cout << "# This code was produced by the CERI Compiler" << endl;
    ReadChar();
    Program();
    cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top" << endl;
    cout << "\tret\t\t\t# Return from main function" << endl;
    if(cin.get(current)){
        cerr << "Caractères en trop à la fin du programme : [" << current << "]";
        Error(".");
    }
}