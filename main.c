#include <stdio.h>
#include <stdlib.h>

#include "gen.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: gcc-- <file.c>\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }

    long sz = ftell(file);
    if (sz < 0) {
        perror("ftell");
        exit(EXIT_FAILURE);
    }

    rewind(file);

    char *buf = malloc((size_t)sz + 1);
    if (!buf) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    size_t read = fread(buf, 1, (size_t)sz, file);
    if (read != (size_t)sz) {
        fprintf(stderr, "Error: Failed to read file\n");
        exit(EXIT_FAILURE);
    }

    buf[sz] = '\0';
    fclose(file);

    Lexer lexer;
    lexer_init(&lexer, buf);

    Parser parser;
    parser_init(&parser, &lexer);

    Program *program = parse(&parser);
    gen(stdout, program);
    parser_free(program);
    free(buf);
    return EXIT_SUCCESS;
}
