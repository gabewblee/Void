#include <parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Expr *parse_expr(Parser *parser);
static Stmt *parse_stmt(Parser *parser);
static Stmt *parse_expr_stmt(Parser *parser);
static Block *parse_block(Parser *parser);

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

static Expr *parse_primary_expr(Parser *parser) {
    /* primary -> number | id | (expr) */
    if (parser->lookahead.type == TOKEN_NUM) {
        long num = parser->lookahead.num;
        match(parser, TOKEN_NUM);
        return ast_build_integer_expr_node(num);
    }

    if (parser->lookahead.type == TOKEN_ID) {
        char *name = copy_token_lexeme(parser->lookahead);
        match(parser, TOKEN_ID);
        return ast_build_id_expr_node(name);
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

static Expr *parse_unary_expr(Parser *parser) {
    /* unary -> (+ | - | !) unary | primary */
    TokenType op = parser->lookahead.type;
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_NOT) {
        match(parser, op);
        return ast_build_unary_expr_node(op, parse_unary_expr(parser));
    }

    return parse_primary_expr(parser);
}

static Expr *parse_multiplicative_expr(Parser *parser) {
    /* multiplicative -> unary ((* | /) unary)* */
    Expr *left = parse_unary_expr(parser);
    while (parser->lookahead.type == TOKEN_MULT || parser->lookahead.type == TOKEN_DIV) {
        TokenType op = parser->lookahead.type;
        match(parser, parser->lookahead.type);
        
        Expr *right = parse_unary_expr(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Expr *parse_additive_expr(Parser *parser) {
    /* additive -> multiplicative ((+ | -) multiplicative)* */
    Expr *left = parse_multiplicative_expr(parser);
    while (parser->lookahead.type == TOKEN_PLUS || parser->lookahead.type == TOKEN_MINUS) {
        TokenType op = parser->lookahead.type;
        match(parser, op);

        Expr *right = parse_multiplicative_expr(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Expr *parse_relational_expr(Parser *parser) {
    /* relational -> additive ((< | <= | > | >=) additive)* */
    Expr *left = parse_additive_expr(parser);
    while (parser->lookahead.type == TOKEN_LESS    || 
           parser->lookahead.type == TOKEN_LEQ     ||
           parser->lookahead.type == TOKEN_GREATER ||
           parser->lookahead.type == TOKEN_GEQ) {
        TokenType op = parser->lookahead.type;
        match(parser, parser->lookahead.type);
        
        Expr *right = parse_additive_expr(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Expr *parse_equality_expr(Parser *parser) {
    /* equality -> relational ((== | !=) relational)* */
    Expr *left = parse_relational_expr(parser);
    while (parser->lookahead.type == TOKEN_EQEQ || parser->lookahead.type == TOKEN_NEQ) {
        TokenType op = parser->lookahead.type;
        match(parser, parser->lookahead.type);
        
        Expr *right = parse_relational_expr(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Expr *parse_land_expr(Parser *parser) {
    /* land -> equality (&& equality)* */
    Expr *left = parse_equality_expr(parser);
    while (parser->lookahead.type == TOKEN_ANDAND) {
        TokenType op = parser->lookahead.type;
        match(parser, parser->lookahead.type);
        Expr *right = parse_equality_expr(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Expr *parse_lor_expr(Parser *parser) {
    /* lor -> land (|| land)* */
    Expr *left = parse_land_expr(parser);
    while (parser->lookahead.type == TOKEN_OROR) {
        TokenType op = parser->lookahead.type;
        match(parser, parser->lookahead.type);
        Expr *right = parse_land_expr(parser);
        left = ast_build_binary_expr_node(op, left, right);
    }

    return left;
}

static Expr *parse_assign_expr(Parser *parser) {
    /* assign -> lor (= assign)? */
    Expr *expr = parse_lor_expr(parser);
    if (parser->lookahead.type == TOKEN_EQ) {
        if (expr->type != EXPR_ID) {
            fprintf(stderr, "Error: Expected expression type '%d', got '%d'.\n", EXPR_ID, expr->type);
            exit(EXIT_FAILURE);
        }

        match(parser, TOKEN_EQ);
        Expr *val = parse_assign_expr(parser);
        return ast_build_assign_expr_node(expr, val);
    }

    return expr;
}

static Expr *parse_expr(Parser *parser) {
    /* expr -> assign */
    return parse_assign_expr(parser);
}

static Stmt *parse_ret_stmt(Parser *parser) {
    /* ret -> return expr; */
    match(parser, TOKEN_RET);
    Stmt *stmt = ast_build_ret_stmt_node(parse_expr(parser));
    match(parser, TOKEN_SEMICOLON);
    return stmt;
}

static Stmt *parse_decl_stmt(Parser *parser) {
    /* decl_stmt -> int id (= expr)?; */
    match(parser, TOKEN_INT);
    Token ident = parser->lookahead;
    match(parser, TOKEN_ID);
    char *name = copy_token_lexeme(ident);
    if (parser->lookahead.type == TOKEN_EQ) {
        match(parser, TOKEN_EQ);
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

static Stmt *parse_if_stmt(Parser *parser) {
    /* if_stmt -> if (expr) stmt (else stmt)?*/
    match(parser, TOKEN_IF);
    match(parser, TOKEN_LPAREN);
    Expr *condition = parse_expr(parser);
    match(parser, TOKEN_RPAREN);
    Stmt *then_branch = parse_stmt(parser);
    Stmt *else_branch = NULL;
    if (parser->lookahead.type == TOKEN_ELSE) {
        match(parser, TOKEN_ELSE);
        else_branch = parse_stmt(parser);
    }
    
    return ast_build_if_stmt_node(condition, then_branch, else_branch);
}

static Stmt *parse_block_stmt(Parser *parser) {
    return ast_build_block_stmt_node(parse_block(parser));
}

static Stmt *parse_while_stmt(Parser *parser) {
    /* while_stmt -> while (cond) body */
    match(parser, TOKEN_WHILE);
    match(parser, TOKEN_LPAREN);
    Expr *cond = parse_expr(parser);
    match(parser, TOKEN_RPAREN);
    Stmt *body = parse_stmt(parser);
    return ast_build_while_stmt_node(cond, body);
}

static Stmt *parse_for_init(Parser *parser) {
    /* for_init -> decl_stmt | expr_stmt */
    if (parser->lookahead.type == TOKEN_INT)
        return parse_decl_stmt(parser);

    return parse_expr_stmt(parser);
}

static Stmt *parse_for_stmt(Parser *parser) {
    /* for_stmt -> for (for_init expr ; expr) stmt */
    match(parser, TOKEN_FOR);
    match(parser, TOKEN_LPAREN);
    Stmt *init = parse_for_init(parser);
    Expr *cond = parse_expr(parser);
    match(parser, TOKEN_SEMICOLON);
    Expr *inc = parse_expr(parser);
    match(parser, TOKEN_RPAREN);
    Stmt *body = parse_stmt(parser);
    return ast_build_for_stmt_node(init, cond, inc, body);
}

static Stmt *parse_break_stmt(Parser *parser) {
    /* break_stmt -> break; */
    match(parser, TOKEN_BREAK);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_break_stmt_node();
}

static Stmt *parse_continue_stmt(Parser *parser) {
    /* continue_stmt -> continue; */
    match(parser, TOKEN_CONTINUE);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_continue_stmt_node();
}

static Stmt *parse_expr_stmt(Parser *parser) {
    /* expr_stmt -> expr; */
    Expr *expr = parse_expr(parser);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_expr_stmt_node(expr);
}

static Stmt *parse_stmt(Parser *parser) {
    /* stmt -> ret_stmt | decl_stmt | if_stmt | block_stmt | while_stmt | for_stmt | break_stmt | continue_stmt | expr_stmt */
    switch (parser->lookahead.type) {
    case TOKEN_RET:
        return parse_ret_stmt(parser);
    case TOKEN_INT:
        return parse_decl_stmt(parser);
    case TOKEN_IF:
        return parse_if_stmt(parser);
    case TOKEN_LBRACE:
        return parse_block_stmt(parser);
    case TOKEN_WHILE:
        return parse_while_stmt(parser);
    case TOKEN_FOR:
        return parse_for_stmt(parser);
    case TOKEN_BREAK:
        return parse_break_stmt(parser);
    case TOKEN_CONTINUE:
        return parse_continue_stmt(parser);
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
    match(parser, TOKEN_ID);
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
