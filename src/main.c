#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <argp.h>

#include "ast.h"
#include "assemble.h"

extern int yydebug;
extern FILE* yyin;
extern int yyparse(ast_t** _result);

const char *argp_program_version = "DCPU-16 assembler 0.1";
const char *argp_program_bug_address = "<github.com/Stenodyon/dcpu16-as/issues>";

static char doc[] =
    "DCPU16 specification 1.7 assembler";

static char args_doc[] =
    "DASM_FILE";

static struct argp_option options[] =
{
    {"output", 'o', "FILE", 0, "Output to FILE instead of out.bin"},
    { 0 }
};

struct arguments
{
    char *input_file;
    char *output_file;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);
void write_binary(const char * filename, bin_buffer_t* buffer);

static struct argp argp = { options, parse_opt, args_doc, doc };

#ifndef _TESTING
int main(int argc, char ** argv)
{
    struct arguments arguments;
    arguments.output_file = "out.bin";
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    ast_t* result = NULL;

#ifdef _DEBUG
    yydebug = 1;
#endif

    yyin = fopen(arguments.input_file, "r");

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

    write_binary(arguments.output_file, buffer);

    buffer_destroy(buffer);
}
#endif

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    switch (key)
    {
    case 'o':
        arguments->output_file = arg;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 1)
            argp_usage(state);
        arguments->input_file = arg;
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 1)
            argp_usage(state);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

void write_binary(const char * filename, bin_buffer_t* buffer)
{
    FILE * file = fopen(filename, "wb");

    fwrite((void*)buffer->data, sizeof(uint16_t), buffer->size, file);

    fclose(file);
}
