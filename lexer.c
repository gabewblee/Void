#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static inline Token tokenize(TokenType type, char *start, int len) {
    return (Token){
        .type  = type,
        .start = start,
        .len   = len
    };
}

static inline int match(char* start, int len, char *keyword) {
    return (len == (int)strlen(keyword)) && strncmp(start, keyword, len) == 0;
}

static inline void handle_blank(Lexer *lexer) {
    while (isspace((unsigned char)*lexer->peek))
        lexer->peek++;
}

static Token handle_ident(Lexer *lexer) {
    char *start = lexer->peek;
    while (isalpha((unsigned char)*lexer->peek) || *lexer->peek == '_')
        lexer->peek++;

    int len = lexer->peek - start;
    if (match(start, len, "int"))
        return tokenize(TOKEN_INT, start, len);

    if (match(start, len, "return"))
        return tokenize(TOKEN_RETURN, start, len);

    return tokenize(TOKEN_IDENTIFIER, start, len);
}

static Token handle_num(Lexer *lexer) {
    char *start = lexer->peek; long num = 0;
    while (isdigit((unsigned char)*lexer->peek)) {
        num = num * 10 + (*lexer->peek - '0');
        lexer->peek++;
    }

    Token token = tokenize(TOKEN_NUMBER, start, (int)(lexer->peek - start));
    token.num = num;
    return token;
}

Token lexer_get_nxt_token(Lexer *lexer) {
    handle_blank(lexer);
    char *peek = lexer->peek;

    /* The next token is an EOF */
    char c = *peek;
    if (c == '\0')
        return tokenize(TOKEN_EOF, peek, 0);

    /* The next token is an identifier */
    if (isalpha((unsigned char)c) || c == '_')
        return handle_ident(lexer);

    /* The next token is a number */
    if (isdigit((unsigned char)c))
        return handle_num(lexer);

    lexer->peek++;
    switch(c) {
    case '(':
        return tokenize(TOKEN_LPAREN, peek, 1);
    case ')':
        return tokenize(TOKEN_RPAREN, peek, 1);
    case '{':
        return tokenize(TOKEN_LBRACE, peek, 1);
    case '}':
        return tokenize(TOKEN_RBRACE, peek, 1);
    case ';':
        return tokenize(TOKEN_SEMICOLON, peek, 1);
    case '+':
        return tokenize(TOKEN_PLUS, peek, 1);
    case '-':
        return tokenize(TOKEN_MINUS, peek, 1);
    case '*':
        return tokenize(TOKEN_STAR, peek, 1);
    case '/':
        return tokenize(TOKEN_SLASH, peek, 1);
    default:
        fprintf(stderr,"Error: Unexpected character '%c'\n", c);
        exit(EXIT_FAILURE);
    }
}

void lexer_init(Lexer *lexer, char *src) {
    lexer->peek = src;
}
