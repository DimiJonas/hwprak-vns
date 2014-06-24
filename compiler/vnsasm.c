
#include <string.h>
#include <unistd.h>

#include "vnsasm.h"

typedef struct _configuration {
    FILE *outfile_d;
    char *outfile_name;
    char *infile_name;
    uint8_t verbose_mode;
} configuration;

configuration config;

void yyerror(char *error)
{
    fprintf(stderr, "%s near line %i\n", error, yylineno);
    exit(EXIT_ERROR);
}

void write_byte(uint8_t byte)
{
    fwrite((void*)&byte, 1, 1, config.outfile_d);
}

void prc_simple_instr(uint8_t ins)
{
    if(config.verbose_mode) {
        printf("Compiling instruction %x.\n", ins);
    }

    write_byte(ins);
}

void prc_addr_instr(uint8_t ins, uint8_t addr)
{
    if(config.verbose_mode) {
        printf("Compiling instruction %x with address arg %x.\n", ins, addr);
    }

    write_byte(ins);
    write_byte(addr);
}

void prc_const_instr(uint8_t ins, uint8_t c)
{
    if(config.verbose_mode) {
        printf("Compiling instruction %x with constant arg %x.\n", ins, c);
    }

    write_byte(ins);
    write_byte(c);
}

int compile(void)
{
    if(NULL == (yyin = fopen(config.infile_name, "r"))) {
        perror(config.infile_name);
        return EXIT_ERROR;
    }

    /* FIXME: write to temporary file and rename on success */
    if(NULL == (config.outfile_d = fopen(config.outfile_name, "w+"))) {
        perror(config.outfile_name);
        return EXIT_ERROR;
    }

    fprintf(stdout, "Compiling %s into %s...\n",
            config.infile_name, config.outfile_name);
    if(0 != yyparse()) {
        return EXIT_ERROR;
    }

    fclose(yyin);
    fclose(config.outfile_d);

    return EXIT_OK;
}

void print_usage(char *pname)
{
    printf("\nUsage: %s [-h] | [-v] [-o <outfile>] <asmfile>\n\n", pname);
    printf("  -h             Show this help text.\n");
    printf("  -v             Turn on verbose output.\n");
    printf("  -o <outfile>   Write compiled program to <outfile>.\n");
    printf("\n");
}

char *simple_basename(char *path)
{
    char *basename;

    if(NULL != (basename = rindex(path, '/'))) {
        return basename + 1;
    }

    return path;
}

int main(int argc, char **argv)
{
    unsigned int opt;
    char *process_name = simple_basename(argv[0]);

    fprintf(stdout, "TUD HWPRAK Von-Neumann Simulator - ASM Compiler\n");

    /* initialize default configuration */
    config.outfile_d = NULL;
    config.outfile_name = NULL;
    config.infile_name = NULL;
    config.verbose_mode = FALSE;

    /* parse comdline arguments */
    while((opt = getopt(argc, argv, "ho:v")) != -1) {
        switch(opt) {
            case 'h':
                print_usage(process_name);
                return EXIT_OK;
            case 'v':
                config.verbose_mode = TRUE;
                break;
            case 'o':
                config.outfile_name = strdup(optarg);
                break;
            default:
                print_usage(process_name);
                return EXIT_OK;
        }
    }

    if(optind >= argc) {
        print_usage(process_name);
        return EXIT_OK;
    }

    config.infile_name = strdup(argv[optind]);

    return compile();
}
