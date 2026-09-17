#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Expr *parse_expr(Parser *parser);

static void advance(Parser *parser) {
    parser->lookahead = lexer_get_nxt_token(parser->lexer);
}

static void match(Parser *parser, TokenType type) {
    if (parser->lookahead.type != type) {
        fprintf(stderr, "Error: Expected '%d', got '%d'.\n", type, parser->lookahead.type);
        exit(EXIT_FAILURE);
    }
    
    advance(parser);
}

static char *copy_token_lexeme(Token token) {
    char *lexeme = malloc(token.len + 1);
    if (!lexeme) {
        fprintf(stderr, "Error: Failed to copy lexeme. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(lexeme, token.start, token.len);
    lexeme[token.len] = '\0';
    return lexeme;
}

static Expr *parse_primary(Parser *parser) {
    /* primary -> number | id | (expr) */
    if (parser->lookahead.type == TOKEN_NUMBER) {
        long num = parser->lookahead.num;
        match(parser, TOKEN_NUMBER);
        return ast_build_integer_expr_node(num);
    }

    if (parser->lookahead.type == TOKEN_IDENTIFIER) {
        char *name = copy_token_lexeme(parser->lookahead);
        match(parser, TOKEN_IDENTIFIER);
        return ast_build_identifier_expr_node(name);
    }

    if (parser->lookahead.type == TOKEN_LPAREN) {
        match(parser, TOKEN_LPAREN);
        Expr *primary = parse_expr(parser);
        match(parser, TOKEN_RPAREN);
        return primary;
    }

    fprintf(stderr, "Error: Expected primary, got '%d'.\n", parser->lookahead.type);
    exit(EXIT_FAILURE);
}

static Expr *parse_unary(Parser *parser) {
    /* unary -> (+ | -) unary | primary */
    TokenType op = parser->lookahead.type;
    if (op == TOKEN_PLUS || op == TOKEN_MINUS) {
        match(parser, op);
        return ast_build_unary_expr_node(op, parse_unary(parser));
    }

    return parse_primary(parser);
}

static Expr *parse_term(Parser *parser) {
    /* term -> unary ((* | /) unary)* */
    Expr *left = parse_unary(parser);
    while (parser->lookahead.type == TOKEN_STAR || parser->lookahead.type == TOKEN_SLASH) {
        TokenType op = parser->lookahead.type;
        match(parser, op);

        Expr *right = parse_unary(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }
    
    return left;
}

static Expr *parse_expr(Parser *parser) {
    /* expr -> term ((+ | -) term)* */
    Expr *left = parse_term(parser);
    while (parser->lookahead.type == TOKEN_PLUS || parser->lookahead.type == TOKEN_MINUS) {
        TokenType op = parser->lookahead.type;
        match(parser, op);

        Expr *right = parse_term(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Stmt *parse_return_stmt(Parser *parser) {
    /* ret -> return expr; */
    match(parser, TOKEN_RETURN);
    Stmt *stmt = ast_build_return_stmt_node(parse_expr(parser));
    match(parser, TOKEN_SEMICOLON);
    return stmt;
}

static Stmt *parse_decl_stmt(Parser *parser) {
    /* decl_stmt -> int id (= expr)?; */
    match(parser, TOKEN_INT);
    char *name = copy_token_lexeme(parser->lookahead);
    match(parser, TOKEN_IDENTIFIER);
    if (parser->lookahead.type == TOKEN_EQUAL) {
        match(parser, TOKEN_EQUAL);
        Expr *initializer = parse_expr(parser);
        match(parser, TOKEN_SEMICOLON);
        return ast_build_decl_stmt_node(name, initializer);
    }

    if (parser->lookahead.type == TOKEN_SEMICOLON) {
        match(parser, TOKEN_SEMICOLON);
        return ast_build_decl_stmt_node(name, NULL);
    }

    fprintf(stderr, "Error: Failed to parse token type '%d'.\n", parser->lookahead.type);
    exit(EXIT_FAILURE);
}

static Stmt *parse_expr_stmt(Parser *parser) {
    /* expr_stmt -> expr; */
    Expr *expr = parse_expr(parser);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_expr_stmt_node(expr);
}

static Stmt *parse_stmt(Parser *parser) {
    /* stmt -> ret_stmt | decl_stmt | expr_stmt */
    switch (parser->lookahead.type) {
    case TOKEN_RETURN:
        return parse_return_stmt(parser);
    case TOKEN_INT:
        return parse_decl_stmt(parser);
    default:
        return parse_expr_stmt(parser);
    }
}

static Block *parse_block(Parser *parser) {
    /* block -> { stmt* } */

    match(parser, TOKEN_LBRACE);

    Stmt **stmts = NULL;
    int cnt = 0, cap = 0;
    while (parser->lookahead.type != TOKEN_RBRACE) {
        if (parser->lookahead.type == TOKEN_EOF) {
            fprintf(stderr, "Error: Expected '}', got EOF.\n");
            exit(EXIT_FAILURE);
        }

        if (cnt == cap) {
            /* Expand twice in size */
            cap = cap ? cap << 1 : 1;
            Stmt **tmp = realloc(stmts, cap * sizeof(Stmt *));
            if (!tmp) {
                fprintf(stderr, "Error: Failed to build statement array. Out of memory.\n");
                exit(EXIT_FAILURE);
            }
            stmts = tmp;
        }

        stmts[cnt++] = parse_stmt(parser);
    }

    match(parser, TOKEN_RBRACE);
    return ast_build_block_node(stmts, cnt);
}

static Function *parse_function(Parser *parser) {
    /* function -> int id() block */

    match(parser, TOKEN_INT);

    Token lookahead = parser->lookahead;
    match(parser, TOKEN_IDENTIFIER);
    char *name = copy_token_lexeme(lookahead);

    match(parser, TOKEN_LPAREN);
    match(parser, TOKEN_RPAREN);
    
    Block *block = parse_block(parser);
    return ast_build_function_node(name, block);
}

Program *parse(Parser *parser) {
    /* program -> function EOF */
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
