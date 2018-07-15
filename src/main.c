#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ast.h"
#include "assemble.h"

extern int yydebug;
extern FILE* yyin;
extern int yyparse(ast_t** _result);

void write_binary(char * filename, bin_buffer_t* buffer);

int main(int argc, char ** argv)
{
    ast_t* result = NULL;

    if (argc != 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        exit(-1);
    }

#ifdef _DEBUG
    yydebug = 1;
#endif

    yyin = fopen(argv[1], "r");

    if (yyin == NULL)
    {
        perror(argv[1]);
        exit(-1);
    }

    yyparse(&result);
    fclose(yyin);

    if (!result)
        exit(-1);

    bin_buffer_t* buffer = assemble(result);
    ast_destroy(result);

    write_binary("out.bin", buffer);

    buffer_destroy(buffer);
}

void write_binary(char * filename, bin_buffer_t* buffer)
{
    FILE * file = fopen(filename, "wb");

    fwrite((void*)buffer->data, sizeof(uint16_t), buffer->size, file);

    fclose(file);
}
