%{
#include <stdio.h>
#include <string.h>

#include "ast.h"

#include "dasm.tab.h"

// For some reason the standard strdup function returns an innaccessible
// memory address
char * strdup(char * to_copy);

extern int yylex();
%}

%option noyywrap yylineno

%x COMMENT STRING

%%

;              BEGIN(COMMENT);
<COMMENT>[^\n] { }
<COMMENT>\n    { BEGIN(0); return NEWLINE; }

\"             BEGIN(STRING);
<STRING>[^\"]  {
    yylval.sval = strdup(yytext);
    return STRING_LIT;
}
<STRING>\"     BEGIN(0);

[ \t]          { }
\n             { return NEWLINE; }
\[             { return SQB_OPEN; }
\]             { return SQB_CLOSE; }
\+             { return PLUS; }
,              { return COLON; }
[0-9]+         { yylval.ival = strtol(yytext, NULL, 10); return VALUE; }
0x[0-9a-fA-F]+ { yylval.ival = strtol(yytext, NULL, 16); return VALUE; }

A              { return A; }
B              { return B; }
C              { return C; }
X              { return X; }
Y              { return Y; }
Z              { return Z; }
I              { return I; }
J              { return J; }
PC             { return PC; }
SP             { return SP; }
EX             { return EX; }
IA             { return IA; }

PUSH           { return PUSH; }
POP            { return POP; }

(?i:SET)       { return SET; }

(?i:ADD)       { return ADD; }
(?i:SUB)       { return SUB; }
(?i:MUL)       { return MUL; }
(?i:MLI)       { return MLI; }
(?i:DIV)       { return DIV; }
(?i:DVI)       { return DVI; }
(?i:MOD)       { return MOD; }
(?i:MDI)       { return MDI; }

(?i:AND)       { return AND; }
(?i:BOR)       { return BOR; }
(?i:XOR)       { return XOR; }
(?i:SHR)       { return SHR; }
(?i:ASR)       { return ASR; }
(?i:SHL)       { return SHL; }

(?i:IFB)       { return IFB; }
(?i:IFC)       { return IFC; }
(?i:IFE)       { return IFE; }
(?i:IFN)       { return IFN; }
(?i:IFG)       { return IFG; }
(?i:IFA)       { return IFA; }
(?i:IFL)       { return IFL; }
(?i:IFU)       { return IFU; }

(?i:ADX)       { return ADX; }
(?i:SBX)       { return SBX; }

(?i:STI)       { return STI; }
(?i:STD)       { return STD; }

(?i:JSR)       { return JSR; }

(?i:INT)       { return INT; }
(?i:IAG)       { return IAG; }
(?i:IAS)       { return IAS; }
(?i:RFI)       { return RFI; }
(?i:IAQ)       { return IAQ; }

(?i:HWN)       { return HWN; }
(?i:HWQ)       { return HWQ; }
(?i:HWI)       { return HWI; }

(?i:DBP)       { return DBP; }
(?i:DBH)       { return DBH; }

(?i:DAT)       { return DAT; }
(?i:RES)       { return RES; }

[a-zA-Z_.][a-zA-Z0-9_.]* {
    yylval.sval = strdup(yytext);
    return LABEL_NAME;
}
:[a-zA-Z_.][a-zA-Z0-9_.]* {
    yylval.sval = strdup(yytext + 1); // + 1 to remove the leading :
    return LABEL;
}

%%

#if 0
int main(void)
{
    while (yylex());
    return 0;
}
#endif

char * strdup(char * to_copy)
{
    char * ptr = (char*)malloc((strlen(to_copy) + 1) * sizeof(char));
    strcpy(ptr, to_copy);
    return ptr;
}
