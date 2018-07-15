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

%union {
    uint16_t ival;
    char * sval;
    operand_t* operand_val;
}

// Random
%token NEWLINE
%token SQB_OPEN SQB_CLOSE
%token PLUS

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

%%

dasm:
    LABEL { printf("Label: '%s'\n", $1); fflush(stdout); }
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
        | '[' register ']' {
                    operand_t* reg = $2;
                    reg->id += 0x08;
                    $$ = reg;
                  }
        | '[' register '+' VALUE ']' {
                    operand_t * reg = $2;
                    reg->id += 0x10;
                    reg->nextword = $4;
                    $$ = reg;
                  }
        | '[' SP ']' { $$ = ast_make_operand(0x19, 0); }
        | '[' SP '+' VALUE ']' { $$ = ast_make_operand(0x1A, $4); }
        | SP { $$ = ast_make_operand(0x1B, 0); }
        | PC { $$ = ast_make_operand(0x1C, 0); }
        | EX { $$ = ast_make_operand(0x1D, 0); }
        | '[' VALUE ']' { $$ = ast_make_operand(0x1E, $2); }
        | LABEL_NAME {
            operant_t* operand = ast_make_operand(-1, 0); 
            operand->label_name = $1;
            $$ = operand;
        }
        ;

readable_operand:
                ;

writable_operand:
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
