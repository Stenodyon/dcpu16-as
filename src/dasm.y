%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ast.h"

extern int yylex();
extern int yyparse();
extern FILE *yyin;

void yyerror(const char *s);
%}

%define parse.error verbose

%union {
    uint16_t ival;
    char * sval;
    operand_t* operand_val;
    struct ast_statement* stmt_val;
    ast_t* ast_val;
}

// Random
%token NEWLINE END
%token SQB_OPEN SQB_CLOSE
%token PLUS COLON

// Registers
%token A B C X Y Z I J
%token PC SP EX IA
%token PUSH POP

// Instructions
%token SET

// Values
%token <ival> VALUE;
%token <sval> LABEL;
%token <sval> LABEL_NAME;

%type <operand_val> register;
%type <operand_val> any_operand;
%type <operand_val> readable_operand;
%type <operand_val> writable_operand;

%type <stmt_val> instruction;
%type <stmt_val> statement;

%type <ast_val> dasm;

%%

dasm: END { $$ = ast_make(); }
    | NEWLINE dasm { $$ = $2; }
    | statement NEWLINE dasm {
        ast_t* ast = $3;
        ast_append(ast, $1);
        $$ = ast;
    }
    ;

register:
          A { $$ = ast_make_operand(0x00, 0); }
        | B { $$ = ast_make_operand(0x01, 0); }
        | C { $$ = ast_make_operand(0x02, 0); }
        | X { $$ = ast_make_operand(0x03, 0); }
        | Y { $$ = ast_make_operand(0x04, 0); }
        | Z { $$ = ast_make_operand(0x05, 0); }
        | I { $$ = ast_make_operand(0x06, 0); }
        | J { $$ = ast_make_operand(0x07, 0); }
        ;

any_operand:  register { $$ = $1; }
        | SQB_OPEN register SQB_CLOSE {
                    operand_t* reg = $2;
                    reg->id += 0x08;
                    $$ = reg;
                  }
        | SQB_OPEN register PLUS VALUE SQB_CLOSE {
                    operand_t * reg = $2;
                    reg->id += 0x10;
                    reg->nextword = $4;
                    $$ = reg;
                  }
        | SQB_OPEN SP SQB_CLOSE { $$ = ast_make_operand(0x19, 0); }
        | SQB_OPEN SP PLUS VALUE SQB_CLOSE { $$ = ast_make_operand(0x1A, $4); }
        | SP { $$ = ast_make_operand(0x1B, 0); }
        | PC { $$ = ast_make_operand(0x1C, 0); }
        | EX { $$ = ast_make_operand(0x1D, 0); }
        | SQB_OPEN VALUE SQB_CLOSE { $$ = ast_make_operand(0x1E, $2); }
        | LABEL_NAME {
            operand_t* operand = ast_make_operand(-1, 0); 
            operand->label_name = $1;
            $$ = operand;
        }
        ;

readable_operand: any_operand { $$ = $1; }
                | POP { $$ = ast_make_operand(0x18, 0); }
                | VALUE { $$ = ast_make_operand(0x1F, $1); }
                ;

writable_operand: any_operand { $$ = $1; }
                | PUSH { $$ = ast_make_operand(0x18, 0); }
                ;

instruction: SET writable_operand COLON readable_operand {
            $$ = ast_makeSET($2, $4);
           }
           ;

statement: instruction { $$ = $1; }
         | LABEL { $$ = ast_make_label($1); }
         ;

%%

int main(void)
{
    yyparse();
}

void yyerror(const char * s)
{
    printf("Parse error: %s\n", s);
    exit(-1);
}
