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

%option noyywrap

%%

[ \t]        { }
\n           { return NEWLINE; }
<<EOF>>      { return END; }
\[           { return SQB_OPEN; }
\]           { return SQB_CLOSE; }
\+           { return PLUS; }
,            { return COLON; }
(0x)?[0-9]+  { yylval.ival = atoi(yytext); return VALUE; }

A            { return A; }
B            { return B; }
C            { return C; }
X            { return X; }
Y            { return Y; }
Z            { return Z; }
I            { return I; }
J            { return J; }
PC           { return PC; }
SP           { return SP; }
EX           { return EX; }
IA           { return IA; }

PUSH         { return PUSH; }
POP          { return POP; }

(?i:SET)     { return SET; }

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
