#pragma once

typedef enum {
    /* Identifiers and literals         */
    TOKEN_ID,        /* Identifier */
    TOKEN_NUM,       /* Number     */

    /* Keywords                         */
    TOKEN_INT,       /* int        */
    TOKEN_RET,       /* return     */
    TOKEN_IF,        /* if         */
    TOKEN_ELSE,      /* else       */
    TOKEN_WHILE,     /* while      */

    /* Delimiters                       */
    TOKEN_LPAREN,    /* (          */
    TOKEN_RPAREN,    /* )          */
    TOKEN_LBRACE,    /* {          */
    TOKEN_RBRACE,    /* }          */
    TOKEN_SEMICOLON, /* ;          */

    /* Arithmetic operators             */
    TOKEN_PLUS,      /* +          */
    TOKEN_MINUS,     /* -          */
    TOKEN_MULT,      /* *          */
    TOKEN_DIV,       /* /          */

    /* Assignment and logical operators */
    TOKEN_EQ,        /* =          */
    TOKEN_NOT,       /* !          */

    /* Comparison operators             */
    TOKEN_EQEQ,      /* ==         */
    TOKEN_NEQ,       /* !=         */
    TOKEN_LESS,      /* <          */
    TOKEN_LEQ,       /* <=         */
    TOKEN_GREATER,   /* >          */
    TOKEN_GEQ,       /* >=         */
    TOKEN_EOF        /* EOF        */
} TokenType;

typedef struct {
    TokenType type;  /* Token type   */
    char     *start; /* Token start  */
    int       len;   /* Token length */
    long      num;   /* Token number */
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
