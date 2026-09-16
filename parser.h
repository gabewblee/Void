#pragma once

#include "ast.h"
#include "lexer.h"

typedef struct {
    Lexer *lexer;     /* Current lexer */
    Token  lookahead; /* Current token */
} Parser;

/**
 * parse - Parses the input program.
 * @parser: The parser to parse with.
 * Returns: The parsed program node.
 */
Program *parse(Parser *parser);

/**
 * parser_free - Frees the program's allocated memory.
 * @program: The program to free.
 */
void parser_free(Program *program);

/**
 * parser_init - Initializes @parser with @lexer.
 * @parser: The parser to initialize.
 * @lexer: The lexer to initialize with.
 */
void parser_init(Parser *parser, Lexer *lexer);
