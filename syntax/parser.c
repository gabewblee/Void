#include <parser.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Expr *parse_expr(Parser *parser);
static Stmt *parse_stmt(Parser *parser);
static Stmt *parse_expr_stmt(Parser *parser);
static Block *parse_block(Parser *parser);

static void error(Parser *parser, const char *fmt, ...) {
    Token tok = parser->lookahead;
    fprintf(stderr, "%d:%d: error: ", tok.line, tok.col);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static char *get_token_lexeme(TokenType type) {
    switch (type) {
    case TOKEN_ID:        return "identifier";
    case TOKEN_NUM:       return "number";
    case TOKEN_INT:       return "'int'";
    case TOKEN_RET:       return "'return'";
    case TOKEN_IF:        return "'if'";
    case TOKEN_ELSE:      return "'else'";
    case TOKEN_WHILE:     return "'while'";
    case TOKEN_FOR:       return "'for'";
    case TOKEN_BREAK:     return "'break'";
    case TOKEN_CONTINUE:  return "'continue'";
    case TOKEN_LPAREN:    return "'('";
    case TOKEN_RPAREN:    return "')'";
    case TOKEN_LBRACE:    return "'{'";
    case TOKEN_RBRACE:    return "'}'";
    case TOKEN_SEMICOLON: return "';'";
    case TOKEN_COMMA:     return "','";
    case TOKEN_PLUS:      return "'+'";
    case TOKEN_MINUS:     return "'-'";
    case TOKEN_MULT:      return "'*'";
    case TOKEN_DIV:       return "'/'";
    case TOKEN_EQ:        return "'='";
    case TOKEN_NOT:       return "'!'";
    case TOKEN_ANDAND:    return "'&&'";
    case TOKEN_OROR:      return "'||'";
    case TOKEN_EQEQ:      return "'=='";
    case TOKEN_NEQ:       return "'!='";
    case TOKEN_LESS:      return "'<'";
    case TOKEN_LEQ:       return "'<='";
    case TOKEN_GREATER:   return "'>'";
    case TOKEN_GEQ:       return "'>='";
    case TOKEN_EOF:       return "EOF";
    }
    return "token";
}

static char *cp_token_lexeme(Token token) {
    char *lexeme = malloc(token.len + 1);
    if (!lexeme) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    memcpy(lexeme, token.start, token.len);
    lexeme[token.len] = '\0';
    return lexeme;
}

static void advance(Parser *parser) {
    parser->lookahead = lexer_get_nxt_token(parser->lexer);
}

static void match(Parser *parser, TokenType type) {
    if (parser->lookahead.type != type)
        error(parser, "expected %s, got %s", get_token_lexeme(type), get_token_lexeme(parser->lookahead.type));
    
    advance(parser);
}

static void *grow(void *p, int *cap, size_t elem) {
    *cap = *cap ? *cap << 1 : 1;
    void *tmp = realloc(p, (size_t)*cap * elem);
    if (!tmp) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    return tmp;
}

static Expr **parse_args(Parser *parser, int *argc) {
    Expr **args = NULL; int cap = 0;
    while (parser->lookahead.type != TOKEN_RPAREN) {
        if (*argc == cap)
            args = grow(args, &cap, sizeof(Expr *));

        args[(*argc)++] = parse_expr(parser);
        if (parser->lookahead.type != TOKEN_COMMA)
            break;

        match(parser, TOKEN_COMMA);
    }
    return args;
}

static Expr *parse_primary_expr(Parser *parser) {
    /* primary_expr -> number | id("(" args ")")? | "(" expr ")" */
    if (parser->lookahead.type == TOKEN_NUM) {
        long num = parser->lookahead.num;
        match(parser, TOKEN_NUM);
        return ast_build_integer_expr_node(num);
    }

    if (parser->lookahead.type == TOKEN_ID) {
        char *name = cp_token_lexeme(parser->lookahead);
        match(parser, TOKEN_ID);
        if (parser->lookahead.type == TOKEN_LPAREN) {
            match(parser, TOKEN_LPAREN);
            int argc = 0;
            Expr **args = parse_args(parser, &argc);
            match(parser, TOKEN_RPAREN);
            return ast_build_call_expr_node(name, args, argc);
        }
        return ast_build_id_expr_node(name);
    }

    if (parser->lookahead.type == TOKEN_LPAREN) {
        match(parser, TOKEN_LPAREN);
        Expr *primary_expr = parse_expr(parser);
        match(parser, TOKEN_RPAREN);
        return primary_expr;
    }

    error(parser, "expected expression, got %s", get_token_lexeme(parser->lookahead.type));
    return NULL;
}

static Expr *parse_unary_expr(Parser *parser) {
    /* unary_expr -> ("+" | "-" | "!") unary_expr | primary_expr */
    TokenType op = parser->lookahead.type;
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_NOT) {
        match(parser, op);
        return ast_build_unary_expr_node(op, parse_unary_expr(parser));
    }

    return parse_primary_expr(parser);
}

static Expr *parse_multiplicative_expr(Parser *parser) {
    /* multiplicative_expr -> unary_expr (("*" | "/") unary_expr)* */
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
    /* additive_expr -> multiplicative_expr (("+" | "-") multiplicative_expr)* */
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
    /* relational_expr -> additive_expr (("<" | "<=" | ">" | ">=") additive_expr)* */
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
    /* equality_expr -> relational_expr (("==" | "!=") relational_expr)* */
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
    /* land_expr -> equality_expr ("&&" equality_expr)* */
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
    /* lor_expr -> land_expr ("||" land_expr)* */
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
    /* assign_expr -> lor_expr ("=" assign_expr)? */
    Expr *target = parse_lor_expr(parser);
    if (parser->lookahead.type == TOKEN_EQ) {
        if (target->type != EXPR_ID) {
            switch (target->type) {
            case EXPR_INT:    error(parser, "cannot assign to a number");           break;
            case EXPR_CALL:   error(parser, "cannot assign to a function call");    break;
            case EXPR_UNARY:  error(parser, "cannot assign to a unary expression"); break;
            case EXPR_BINARY: error(parser, "cannot assign to this expression");    break;
            case EXPR_ASSIGN: error(parser, "cannot assign to an assignment");      break;
            default:          error(parser, "cannot assign to this expression");    break;
            }
        }

        match(parser, TOKEN_EQ);
        Expr *val = parse_assign_expr(parser);
        return ast_build_assign_expr_node(target, val);
    }
    return target;
}

static Expr *parse_expr(Parser *parser) {
    /* expr -> assign_expr */
    return parse_assign_expr(parser);
}

static Stmt *parse_ret_stmt(Parser *parser) {
    /* ret_stmt -> "return" expr ";" */
    match(parser, TOKEN_RET);
    Stmt *stmt = ast_build_ret_stmt_node(parse_expr(parser));
    match(parser, TOKEN_SEMICOLON);
    return stmt;
}

static Stmt *parse_decl_stmt(Parser *parser) {
    /* decl_stmt -> "int" id ("=" expr)? ";" */
    match(parser, TOKEN_INT);
    Token id = parser->lookahead;
    match(parser, TOKEN_ID);
    char *name = cp_token_lexeme(id);
    if (parser->lookahead.type == TOKEN_EQ) {
        match(parser, TOKEN_EQ);
        Expr *initializer = parse_expr(parser);
        match(parser, TOKEN_SEMICOLON);
        return ast_build_decl_stmt_node(name, initializer);
    }

    match(parser, TOKEN_SEMICOLON);
    return ast_build_decl_stmt_node(name, NULL);
}

static Stmt *parse_if_stmt(Parser *parser) {
    /* if_stmt -> "if" "(" expr ")" stmt ("else" stmt)?*/
    match(parser, TOKEN_IF);
    match(parser, TOKEN_LPAREN);
    Expr *cond = parse_expr(parser);
    match(parser, TOKEN_RPAREN);
    Stmt *then_branch = parse_stmt(parser);
    Stmt *else_branch = NULL;
    if (parser->lookahead.type == TOKEN_ELSE) {
        match(parser, TOKEN_ELSE);
        else_branch = parse_stmt(parser);
    }
    
    return ast_build_if_stmt_node(cond, then_branch, else_branch);
}

static Stmt *parse_block_stmt(Parser *parser) {
    return ast_build_block_stmt_node(parse_block(parser));
}

static Stmt *parse_while_stmt(Parser *parser) {
    /* while_stmt -> "while" "(" expr ")" stmt */
    match(parser, TOKEN_WHILE);
    match(parser, TOKEN_LPAREN);
    Expr *cond = parse_expr(parser);
    match(parser, TOKEN_RPAREN);
    Stmt *body = parse_stmt(parser);
    return ast_build_while_stmt_node(cond, body);
}

static Stmt *parse_for_init(Parser *parser) {
    /* for_init -> decl_stmt | expr_stmt */
    return parser->lookahead.type == TOKEN_INT ? parse_decl_stmt(parser) : parse_expr_stmt(parser);
}

static Stmt *parse_for_stmt(Parser *parser) {
    /* for_stmt -> "for" "(" for_init expr ";" expr ")" stmt */
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
    /* break_stmt -> "break" ";" */
    match(parser, TOKEN_BREAK);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_break_stmt_node();
}

static Stmt *parse_continue_stmt(Parser *parser) {
    /* continue_stmt -> "continue" ";" */
    match(parser, TOKEN_CONTINUE);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_continue_stmt_node();
}

static Stmt *parse_expr_stmt(Parser *parser) {
    /* expr_stmt -> expr ";" */
    Expr *expr = parse_expr(parser);
    match(parser, TOKEN_SEMICOLON);
    return ast_build_expr_stmt_node(expr);
}

static Stmt *parse_stmt(Parser *parser) {
    /* stmt -> ret_stmt | decl_stmt | if_stmt | block_stmt | while_stmt | for_stmt | break_stmt | continue_stmt | expr_stmt */
    switch (parser->lookahead.type) {
    case TOKEN_RET:      return parse_ret_stmt(parser);
    case TOKEN_INT:      return parse_decl_stmt(parser);
    case TOKEN_IF:       return parse_if_stmt(parser);
    case TOKEN_LBRACE:   return parse_block_stmt(parser);
    case TOKEN_WHILE:    return parse_while_stmt(parser);
    case TOKEN_FOR:      return parse_for_stmt(parser);
    case TOKEN_BREAK:    return parse_break_stmt(parser);
    case TOKEN_CONTINUE: return parse_continue_stmt(parser);
    default:             return parse_expr_stmt(parser);
    }
}

static Block *parse_block(Parser *parser) {
    /* block -> "{" stmt* "}" */
    match(parser, TOKEN_LBRACE);

    Stmt **stmts = NULL; int stmtc = 0, cap = 0;
    while (parser->lookahead.type != TOKEN_RBRACE) {
        if (parser->lookahead.type == TOKEN_EOF)
            error(parser, "expected '}' before EOF");

        if (stmtc == cap)
            stmts = grow(stmts, &cap, sizeof(Stmt *));

        stmts[stmtc++] = parse_stmt(parser);
    }

    match(parser, TOKEN_RBRACE);
    return ast_build_block_node(stmts, stmtc);
}

static char **parse_params(Parser *parser, int *paramc) {
    /* params -> ("int" id ("," "int" id)*)? */
    char **params = NULL; int cap = 0;
    while (parser->lookahead.type != TOKEN_RPAREN) {
        match(parser, TOKEN_INT);
        Token id = parser->lookahead;
        match(parser, TOKEN_ID);
        if (*paramc == cap)
            params = grow(params, &cap, sizeof(char *));

        params[(*paramc)++] = cp_token_lexeme(id);
        if (parser->lookahead.type != TOKEN_COMMA)
            break;

        match(parser, TOKEN_COMMA);
    }
    return params;
}

static Function *parse_function(Parser *parser) {
    /* function -> "int" id "(" params ")" (block | ";") */
    match(parser, TOKEN_INT);

    Token id = parser->lookahead;
    match(parser, TOKEN_ID);
    char *name = cp_token_lexeme(id);

    match(parser, TOKEN_LPAREN);
    int paramc = 0;
    char **params = parse_params(parser, &paramc);
    match(parser, TOKEN_RPAREN);

    /* ";" indicates declaration */
    if (parser->lookahead.type == TOKEN_SEMICOLON) {
        match(parser, TOKEN_SEMICOLON);
        return ast_build_function_node(name, params, paramc, NULL);
    }

    /* Block indicates definition */
    Block *block = parse_block(parser);
    return ast_build_function_node(name, params, paramc, block);
}

Program *parse(Parser *parser) {
    /* program -> function* EOF */
    Function **functions = NULL; int functionc = 0, cap = 0;
    while (parser->lookahead.type != TOKEN_EOF) {
        if (functionc == cap)
            functions = grow(functions, &cap, sizeof(Function *));
        
        functions[functionc++] = parse_function(parser);
    }

    match(parser, TOKEN_EOF);
    return ast_build_program_node(functions, functionc);
}

void parser_free(Program *program) {
    ast_free_program_node(program);
}

void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer     = lexer;
    parser->lookahead = lexer_get_nxt_token(lexer);
}
