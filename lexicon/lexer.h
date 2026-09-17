#pragma once

typedef enum {
    TOKEN_INT,        /* int        */
    TOKEN_RETURN,     /* return     */
    TOKEN_IDENTIFIER, /* identifier */
    TOKEN_NUMBER,     /* number     */
    TOKEN_LPAREN,     /* (          */
    TOKEN_RPAREN,     /* )          */
    TOKEN_LBRACE,     /* {          */
    TOKEN_RBRACE,     /* }          */
    TOKEN_SEMICOLON,  /* ;          */
    TOKEN_PLUS,       /* +          */
    TOKEN_MINUS,      /* -          */
    TOKEN_STAR,       /* *          */
    TOKEN_SLASH,      /* /          */
    TOKEN_EQUAL,      /* =          */
    TOKEN_EOF         /* EOF        */
} TokenType;

typedef struct {
    TokenType  type;  /* Token type   */
    char      *start; /* Token start  */
    int        len;   /* Token length */
    long       num;   /* Token number */
} Token;

typedef struct {
    char *peek; /* Current character */
    int   line; /* Current line      */
} Lexer;

/**
 * lexer_get_nxt_token - Gets the next token from @lexer.
 * @lexer: The lexer to read with.
 * Returns: The next token.
 */
Token lexer_get_nxt_token(Lexer *lexer);

/**
 * lexer_init - Initializes @lexer with @src.
 * @lexer: The lexer to initialize.
 * @src: The input program to initialize with.
 */
void lexer_init(Lexer *lexer, char *src);
