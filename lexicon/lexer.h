#pragma once

typedef enum {
    /* Identifiers and literals         */
    TOKEN_ID,        /* id         */
    TOKEN_NUM,       /* num        */

    /* Keywords                         */
    TOKEN_INT,       /* "int"      */
    TOKEN_RET,       /* "return"   */
    TOKEN_IF,        /* "if"       */
    TOKEN_ELSE,      /* "else"     */
    TOKEN_WHILE,     /* "while"    */
    TOKEN_FOR,       /* "for"      */
    TOKEN_BREAK,     /* "break"    */
    TOKEN_CONTINUE,  /* "continue" */

    /* Delimiters                       */
    TOKEN_LPAREN,    /* "("        */
    TOKEN_RPAREN,    /* ")"        */
    TOKEN_LBRACE,    /* "{"        */
    TOKEN_RBRACE,    /* "}"        */
    TOKEN_SEMICOLON, /* ";"        */
    TOKEN_COMMA,     /* ","        */

    /* Arithmetic operators             */
    TOKEN_PLUS,      /* "+"        */
    TOKEN_MINUS,     /* "-"        */
    TOKEN_MULT,      /* "*"        */
    TOKEN_DIV,       /* "/"        */

    /* Assignment and logical operators */
    TOKEN_EQ,        /* "="        */
    TOKEN_NOT,       /* "!"        */
    TOKEN_ANDAND,    /* "&&"       */
    TOKEN_OROR,      /* "||"       */

    /* Comparison operators             */
    TOKEN_EQEQ,      /* "=="       */
    TOKEN_NEQ,       /* "!="       */
    TOKEN_LESS,      /* "<"        */
    TOKEN_LEQ,       /* "<="       */
    TOKEN_GREATER,   /* ">"        */
    TOKEN_GEQ,       /* ">="       */
    TOKEN_EOF        /* EOF        */
} TokenKind;

typedef struct {
    TokenKind kind;  /* Token kind   */
    char     *start; /* Token start  */
    int       len;   /* Token length */
    long      num;   /* Token number */

    /* Token location information */
    int       line;  /* Token line   */
    int       col;   /* Token column */
} Token;

typedef struct {
    char *peek; /* Current character */
    int   line; /* Current line      */
    int   col;  /* Current column    */
} Lexer;

/**
 * lexer_get_nxt_token - Reads the next token from @lexer.
 * @lexer: The lexer to read from.
 * Returns: The next token from @lexer.
 */
Token lexer_get_nxt_token(Lexer *lexer);

/**
 * lexer_init - Initializes @lexer with @src.
 * @lexer: The lexer to initialize.
 * @src: The source code string.
 */
void lexer_init(Lexer *lexer, char *src);
