#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

static Expr *ast_build_expr_node(ExprType type) {
    Expr *expr = malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    expr->type = type;
    return expr;
}

static void ast_free_expr_node(Expr *expr) {
    if (!expr)
        return;

    switch (expr->type) {
        case EXPR_INTEGER:
            break;

        case EXPR_UNARY:
            ast_free_expr_node(expr->unary.operand);
            break;

        case EXPR_BINARY:
            ast_free_expr_node(expr->binary.left);
            ast_free_expr_node(expr->binary.right);
            break;
    }

    free(expr);
}

static void ast_free_return_stmt_node(ReturnStmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->expr);
    free(stmt);
}

static void ast_free_function_node(Function *function) {
    if (!function)
        return;

    free(function->name);
    ast_free_return_stmt_node(function->body);
    free(function);
}

Expr *ast_build_integer_expr_node(long integer) {
    Expr *expr = ast_build_expr_node(EXPR_INTEGER);
    expr->integer = integer;
    return expr;
}

Expr *ast_build_unary_expr_node(TokenType op, Expr *operand) {
    Expr *expr = ast_build_expr_node(EXPR_UNARY);
    expr->unary.op      = op;
    expr->unary.operand = operand;
    return expr;
}

Expr *ast_build_binary_expr_node(TokenType op, Expr *left, Expr *right) {
    Expr *expr = ast_build_expr_node(EXPR_BINARY);
    expr->binary.op    = op;
    expr->binary.left  = left;
    expr->binary.right = right;
    return expr;
}

ReturnStmt *ast_build_return_stmt_node(Expr *expr) {
    ReturnStmt *stmt = malloc(sizeof(ReturnStmt));
    if (!stmt) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    stmt->expr = expr;
    return stmt;
}

Function *ast_build_function_node(char *name, ReturnStmt *body) {
    Function *function = malloc(sizeof(Function));
    if (!function) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    function->name = name;
    function->body = body;
    return function;
}

Program *ast_build_program_node(Function *function) {
    Program *program = malloc(sizeof(Program));
    if (!program) {
        fprintf(stderr, "Error: Out of memory\n");
        exit(EXIT_FAILURE);
    }

    program->function = function;
    return program;
}

void ast_free_program_node(Program *program) {
    if (!program)
        return;

    ast_free_function_node(program->function);
    free(program);
}