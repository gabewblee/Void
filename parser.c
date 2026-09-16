#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static void advance(Parser *parser) {
    parser->lookahead = lexer_get_nxt_token(parser->lexer);
}

static void match(Parser *parser, TokenType type) {
    if (parser->lookahead.type != type) {
        fprintf(stderr, "Error: Expected '%d', but got '%d'\n", type, parser->lookahead.type);
        exit(EXIT_FAILURE);
    }
    
    advance(parser);
}

static char *copy_token_lexeme(Token token) {
    char *lexeme = malloc(token.len + 1);
    if (!lexeme) {
        fprintf(stderr, "Error: No memory available\n");
        exit(EXIT_FAILURE);
    }

    memcpy(lexeme, token.start, token.len);
    lexeme[token.len] = '\0';
    return lexeme;
}

static Expr *parse_expr(Parser *parser) {
    long num = parser->lookahead.num;
    match(parser, TOKEN_NUMBER);
    return ast_build_integer_expr_node(num);
}

static ReturnStmt *parse_return_stmt(Parser *parser) {
    match(parser, TOKEN_RETURN);
    return ast_build_return_stmt_node(parse_expr(parser));
}

static Function *parse_function(Parser *parser) {
    /* int */
    match(parser, TOKEN_INT);

    /* main */
    Token lookahead = parser->lookahead;
    match(parser, TOKEN_IDENTIFIER);
    char *name = copy_token_lexeme(lookahead);

    /* ( */
    match(parser, TOKEN_LPAREN);
    
    /* ) */
    match(parser, TOKEN_RPAREN);
    
    /* { */
    match(parser, TOKEN_LBRACE);

    /* return */
    ReturnStmt *stmt = parse_return_stmt(parser);

    /* ; */
    match(parser, TOKEN_SEMICOLON);

    /* } */
    match(parser, TOKEN_RBRACE);
    return ast_build_function_node(name, stmt);
}

Program *parse(Parser *parser) {
    /*
     * program -> function EOF
     */
    Program *program = ast_build_program_node(parse_function(parser));
    match(parser, TOKEN_EOF);
    return program;
}

void parser_free(Program *program) {
    ast_free_program_node(program);
}

void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer     = lexer;
    parser->lookahead = lexer_get_nxt_token(lexer);
}
