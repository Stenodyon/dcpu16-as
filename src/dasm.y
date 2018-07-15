%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "ast.h"

#ifdef _DEBUG
#define YYDEBUG 1
#endif

//extern YYLTYPE yylloc;
extern int yylex();
extern int yyparse(ast_t** _result);
extern FILE *yyin;

void yyerror(ast_t ** _result, const char *s);
%}

%define parse.error verbose
%parse-param {ast_t** _result}

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
%token SET STI STD
%token ADD SUB MUL MLI DIV DVI MOD MDI ADX SBX
%token AND BOR XOR SHR ASR SHL
%token IFB IFC IFE IFN IFG IFA IFL IFU
%token JSR
%token INT IAG IAS RFI IAQ
%token HWN HWQ HWI
%token DBP DBH

// Values
%token <ival> VALUE;
%token <sval> LABEL;
%token <sval> LABEL_NAME;

%type <operand_val> register;
%type <operand_val> any_operand;
%type <operand_val> readable_operand;
%type <operand_val> writable_operand;

%type <ival> bin_opcode;
%type <ival> test_opcode;
%type <operand_val> unopcode_w;
%type <operand_val> unopcode_r;

%type <stmt_val> instruction;
%type <stmt_val> statement;

%type <ast_val> dasm;

%%

program: dasm { *_result = $1; };

dasm: %empty { $$ = ast_make(); }
    | statement {
        ast_t* ast = ast_make();
        ast_append(ast, $1);
        $$ = ast;
    }
    | dasm newlines statement {
        ast_t* ast = $1;
        ast_append(ast, $3);
        $$ = ast;
    }
    | dasm newlines { $$ = $1; }
    ;

newlines: NEWLINE
        | newlines NEWLINE
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
            operand_t* operand = ast_make_operand(0x1F, 0); 
            operand->label_name = $1;
            $$ = operand;
        }
        | SQB_OPEN LABEL_NAME SQB_CLOSE {
            operand_t* operand = ast_make_operand(0x1E, 0); 
            operand->label_name = $2;
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

bin_opcode: SET { $$ = 0x01; }
          | ADD { $$ = 0x02; }
          | SUB { $$ = 0x03; }
          | MUL { $$ = 0x04; }
          | MLI { $$ = 0x05; }
          | DIV { $$ = 0x06; }
          | DVI { $$ = 0x07; }
          | MOD { $$ = 0x08; }
          | MDI { $$ = 0x09; }
          | AND { $$ = 0x0A; }
          | BOR { $$ = 0x0B; }
          | XOR { $$ = 0x0C; }
          | SHR { $$ = 0x0D; }
          | ASR { $$ = 0x0E; }
          | SHL { $$ = 0x0F; }
          | ADX { $$ = 0x1A; }
          | SBX { $$ = 0x1B; }
          | STI { $$ = 0x1E; }
          | STD { $$ = 0x1F; }
          ;

test_opcode: IFB { $$ = 0x10; }
           | IFC { $$ = 0x11; }
           | IFE { $$ = 0x12; }
           | IFN { $$ = 0x13; }
           | IFG { $$ = 0x14; }
           | IFA { $$ = 0x15; }
           | IFL { $$ = 0x16; }
           | IFU { $$ = 0x17; }
           ;

unopcode_w: IAG { $$ = ast_make_operand(0x09, 0); }
          | HWN { $$ = ast_make_operand(0x10, 0); }
          ;

unopcode_r: JSR { $$ = ast_make_operand(0x01, 0); }
          | INT { $$ = ast_make_operand(0x08, 0); }
          | IAS { $$ = ast_make_operand(0x0A, 0); }
          | RFI { $$ = ast_make_operand(0x0B, 0); }
          | IAQ { $$ = ast_make_operand(0x0C, 0); }
          | HWQ { $$ = ast_make_operand(0x11, 0); }
          | HWI { $$ = ast_make_operand(0x12, 0); }
          ;

instruction: bin_opcode writable_operand COLON readable_operand {
            $$ = (struct ast_statement*)ast_make_instr($1, $4, $2); }
           | test_opcode readable_operand COLON readable_operand {
            $$ = (struct ast_statement*)ast_make_instr($1, $4, $2); }
           | unopcode_w writable_operand {
            $$ = (struct ast_statement*)ast_make_instr(0, $2, $1); }
           | unopcode_r readable_operand {
            $$ = (struct ast_statement*)ast_make_instr(0, $2, $1); }
           ;

statement: instruction { $$ = $1; }
         | LABEL { $$ = (struct ast_statement*)ast_make_label($1); }
         ;

%%

void yyerror(ast_t ** _result, const char * s)
{
    printf("Parse error: %s\n", s);
    //printf("Line %i, parse error: %s\n", yylloc.first_line, s);
    exit(-1);
}
