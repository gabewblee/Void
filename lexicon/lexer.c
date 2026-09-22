#include <ctype.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char     *lexeme; /* Symbol lexeme */
    TokenKind kind;   /* Symbol kind   */
} Symbol;

static const Symbol symbols[] = {
    /* Multi-character tokens */
    {"&&", TOKEN_ANDAND},
    {"||", TOKEN_OROR},
    {"==", TOKEN_EQEQ},
    {"!=", TOKEN_NEQ},
    {"<=", TOKEN_LEQ},
    {">=", TOKEN_GEQ},

    /* Single-character tokens */
    {"(", TOKEN_LPAREN},
    {")", TOKEN_RPAREN},
    {"{", TOKEN_LBRACE},
    {"}", TOKEN_RBRACE},
    {";", TOKEN_SEMICOLON},
    {",", TOKEN_COMMA},
    {"+", TOKEN_PLUS},
    {"-", TOKEN_MINUS},
    {"*", TOKEN_MULT},
    {"/", TOKEN_DIV},
    {"=", TOKEN_EQ},
    {"!", TOKEN_NOT},
    {"<", TOKEN_LESS},
    {">", TOKEN_GREATER}
};

static inline void advance(Lexer *lexer) {
    if (*lexer->peek == '\0')
        return;

    if (*lexer->peek == '\n') {
        lexer->line++; lexer->col = 1;
    } else
        lexer->col++;

    lexer->peek++;
}

static inline Token tokenize(TokenKind kind, char *start, int len, int line, int col) {
    return (Token){
        .kind  = kind,
        .start = start,
        .len   = len,
        .line  = line,
        .col   = col
    };
}

static inline TokenKind match(char* s, int len) {
    switch (len) {
    case 2:
        if (!memcmp(s, "if", 2)) return TOKEN_IF;
        break;
    case 3:
        if (!memcmp(s, "int", 3)) return TOKEN_INT;
        if (!memcmp(s, "for", 3)) return TOKEN_FOR;
        break;
    case 4:
        if (!memcmp(s, "else", 4)) return TOKEN_ELSE;
        break;
    case 5:
        if (!memcmp(s, "while", 5)) return TOKEN_WHILE;
        if (!memcmp(s, "break", 5)) return TOKEN_BREAK;
        break;
    case 6:
        if (!memcmp(s, "return", 6)) return TOKEN_RET;
        break;
    case 8:
        if (!memcmp(s, "continue", 8)) return TOKEN_CONTINUE;
        break;
    }
    return TOKEN_ID;
}

static inline void handle_blank(Lexer *lexer) {
    while (isspace((unsigned char)*lexer->peek))
        advance(lexer);
}

static inline Token handle_id(Lexer *lexer) {
    char *start = lexer->peek;
    int line = lexer->line, col = lexer->col;

    while (isalnum((unsigned char)*lexer->peek) || *lexer->peek == '_')
        advance(lexer);

    int len = lexer->peek - start;
    return tokenize(match(start, len), start, len, line, col);
}

static inline Token handle_num(Lexer *lexer) {
    char *start = lexer->peek;
    int line = lexer->line, col = lexer->col;

    long num = 0;
    while (isdigit((unsigned char)*lexer->peek)) {
        num = num * 10 + (*lexer->peek - '0');
        advance(lexer);
    }

    int len = lexer->peek - start;
    Token token = tokenize(TOKEN_NUM, start, len, line, col);
    token.num = num;
    return token;
}

static inline Token handle_symbol(Lexer *lexer) {
    char *start = lexer->peek;
    int line = lexer->line, col = lexer->col;

    for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++) {
        size_t len = strlen(symbols[i].lexeme);
        if (!strncmp(start, symbols[i].lexeme, len)) {
            for (size_t j = 0; j < len; j++)
                advance(lexer);

            return tokenize(symbols[i].kind, start, (int)len, line, col);
        }
    }

    fprintf(stderr, "%d:%d: error: unexpected character '%c'\n", line, col, *start);
    exit(EXIT_FAILURE);
}

Token lexer_get_nxt_token(Lexer *lexer) {
    handle_blank(lexer);
    char *peek = lexer->peek, c = *peek;
    if (c == '\0')
        return tokenize(TOKEN_EOF, peek, 0, lexer->line, lexer->col);
    
    if (isalpha((unsigned char)c) || c == '_')
        return handle_id(lexer);
    
    if (isdigit((unsigned char)c))
        return handle_num(lexer);

    return handle_symbol(lexer);
}

void lexer_init(Lexer *lexer, char *src) {
    lexer->peek = src;
    lexer->line = 1;
    lexer->col  = 1;
}
