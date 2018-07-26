#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <argp.h>

#include "ast.h"
#include "assemble.h"
#include "strlist.h"

extern int yydebug;
extern FILE* yyin;
extern int yyparse(ast_t **_result, char *filename);

const char *argp_program_version = "DCPU-16 assembler 0.1";
const char *argp_program_bug_address = "<github.com/Stenodyon/dcpu16-as/issues>";

static char doc[] =
    "DCPU16 specification 1.7 assembler";

static char args_doc[] =
    "DASM_FILES...";

static struct argp_option options[] =
{
    {"verbose", 'v', 0, 0, "Verbose output", 0},
    {"output", 'o', "FILE", 0, "Output to FILE instead of out.bin", 0},
    { 0 }
};

int verbose;

struct arguments
{
    strlist_t *input_files;
    int verbosity;
    char *output_file;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);
void write_binary(const char * filename, bin_buffer_t* buffer);

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

#ifndef _TESTING
int main(int argc, char ** argv)
{
    struct arguments arguments;
    arguments.input_files = strlist_make();
    arguments.output_file = "out.bin";
    arguments.verbosity = 0;
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    verbose = arguments.verbosity;

    ast_t* result = NULL;

#ifdef _DEBUG
    yydebug = 1;
#endif

    for (int i = 0; i < arguments.input_files->size; i++)
    {
        yyin = fopen(arguments.input_files->entries[i], "r");
        if (yyin == NULL)
        {
            fprintf(stderr, "%s:%i Error: ", __FILE__, __LINE__);
            perror(argv[1]);
            exit(-1);
        }
        yyparse(&result, arguments.input_files->entries[i]);
        fclose(yyin);
    }
    strlist_destroy(arguments.input_files);

    if (!result)
        exit(-1);

    bin_buffer_t* buffer = assemble(result);
    if (buffer->size >= 0x10000)
        fprintf(stderr,
                "Warning: the binary is over the 64kB limit by %i bytes\n",
                (buffer->size - 0x10000));

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
    case 'v':
        arguments->verbosity = 1;
        break;
    case 'o':
        arguments->output_file = arg;
        break;
    case ARGP_KEY_ARG:
        strlist_insert(arguments->input_files, arg);
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
