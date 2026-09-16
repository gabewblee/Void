#pragma once

#include "lexer.h"

typedef enum {
    EXPR_INTEGER, /* Integer constant */
    EXPR_UNARY,   /* Unary operation  */
    EXPR_BINARY   /* Binary operaton  */
} ExprType;

typedef struct Expr       Expr;
typedef struct ReturnStmt ReturnStmt;
typedef struct Function   Function;
typedef struct Program    Program;

struct Expr {
    ExprType type; /* Expression type */
    union {
        long integer; /* Expression integer */
        struct {
            TokenType op;      /* Unary operation */
            Expr*     operand; /* Unary operand   */
        } unary;

        struct {
            TokenType op;    /* Binary operation */
            Expr*     left;  /* Left operand     */
            Expr*     right; /* Right operand    */
        } binary;
    };
};

struct ReturnStmt {
    Expr *expr; /* Return value */
};

struct Function {
    char       *name; /* Function name             */
    ReturnStmt *body; /* Function return statement */
};

struct Program {
    Function *function;
};

/**
 * ast_build_integer_expr_node - Builds an integer expression node.
 * @integer: The node's integer value.
 * Returns: The integer expression node.
 */
Expr *ast_build_integer_expr_node(long integer);

/**
 * ast_build_unary_expr_node - Builds a unary expression node.
 * @op: The node's unary operation.
 * @operand: The node's unary operand.
 * Returns: The unary expression node.
 */
Expr *ast_build_unary_expr_node(TokenType op, Expr *operand);

/**
 * ast_build_binary_expr_node - Builds a binary expression node.
 * @op: The node's binary operation.
 * @left: The node's left operand.
 * @right: The node's right operand.
 * Returns: The binary expression node.
 */
Expr *ast_build_binary_expr_node(TokenType op, Expr *left, Expr *right);

/**
 * ast_build_return_stmt_node - Builds a return statement node.
 * @expr: The node's return value.
 * Returns: The return statement node.
 */
ReturnStmt *ast_build_return_stmt_node(Expr *expr);

/**
 * ast_build_function_node - Builds a function node.
 * @name: The function's name.
 * @body: The function's body.
 * Returns: The function node.
 */
Function *ast_build_function_node(char *name, ReturnStmt *body);

/**
 * ast_build_program_node - Builds a program node.
 * @function: The program's entry function.
 * Returns: The program node.
 */
Program *ast_build_program_node(Function *function);

/**
 * ast_free_program_node - Frees the program's allocated memory.
 * @program: The program to free.
 */
void ast_free_program_node(Program *program);