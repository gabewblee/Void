#include <ast.h>
#include <stdio.h>
#include <stdlib.h>

static void ast_free_stmt_node(Stmt *stmt);

static Expr *ast_build_expr_node(ExprType type) {
    Expr *expr = malloc(sizeof(Expr));
    if (!expr) {
        fprintf(stderr, "Error: Failed to build expression node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    expr->type = type;
    return expr;
}

static void ast_free_name(char *s) {
    if (!s)
        return;

    free(s);
}

static void ast_free_expr_node(Expr *expr) {
    if (!expr)
        return;

    switch (expr->type) {
        case EXPR_INT:
            break;
        case EXPR_UNARY:
            ast_free_expr_node(expr->unary.operand);
            break;
        case EXPR_BINARY:
            ast_free_expr_node(expr->binary.left);
            ast_free_expr_node(expr->binary.right);
            break;
        case EXPR_ID:
            ast_free_name(expr->id.name);
            break;
        case EXPR_ASSIGN:
            ast_free_expr_node(expr->assign.target);
            ast_free_expr_node(expr->assign.val);
    }

    free(expr);
}

static void ast_free_ret_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->ret_stmt);
}

static void ast_free_decl_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_name(stmt->decl_stmt.name);
    ast_free_expr_node(stmt->decl_stmt.initializer);
    free(stmt->decl_stmt.symbol);
}

static void ast_free_if_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->if_stmt.cond);
    ast_free_stmt_node(stmt->if_stmt.then_branch);
    ast_free_stmt_node(stmt->if_stmt.else_branch);
}

static void ast_free_expr_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->expr_stmt);
}

static void ast_free_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    switch (stmt->type) {
    case STMT_RETURN:
        ast_free_ret_stmt_node(stmt);
        break;
    case STMT_DECL:
        ast_free_decl_stmt_node(stmt);
        break;
    case STMT_IF:
        ast_free_if_stmt_node(stmt);
        break;
    case STMT_EXPR:
        ast_free_expr_stmt_node(stmt);
        break;
    default:
        fprintf(stderr, "Error: Failed to resolve statement type '%d'.\n", stmt->type);
        exit(EXIT_FAILURE);
    }
    
    free(stmt);
}

static void ast_free_block_node(Block *block) {
    if (!block)
        return;

    for (int i = 0; i < block->cnt; i++)
        ast_free_stmt_node(block->stmts[i]);

    free(block->stmts);
    free(block);
}

static void ast_free_function_node(Function *function) {
    if (!function)
        return;

    free(function->name);
    ast_free_block_node(function->body);
    free(function);
}

Expr *ast_build_integer_expr_node(long integer) {
    Expr *expr = ast_build_expr_node(EXPR_INT);
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

Expr *ast_build_id_expr_node(char *name) {
    Expr *expr = ast_build_expr_node(EXPR_ID);
    expr->id.name   = name;
    expr->id.symbol = NULL;
    return expr;
}

Expr *ast_build_assign_expr_node(Expr *target, Expr *val) {
    Expr *expr = ast_build_expr_node(EXPR_ASSIGN);
    expr->assign.target = target;
    expr->assign.val    = val;
    return expr;
}

Stmt *ast_build_ret_stmt_node(Expr *return_expr) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Error: Failed to build return statement node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    stmt->type     = STMT_RETURN;
    stmt->ret_stmt = return_expr;
    return stmt;
}

Stmt *ast_build_decl_stmt_node(char *name, Expr *initializer) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Error: Failed to build declaration statement node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    stmt->type                  = STMT_DECL;
    stmt->decl_stmt.name        = name;
    stmt->decl_stmt.initializer = initializer;
    stmt->decl_stmt.symbol      = NULL;
    return stmt;
}

Stmt *ast_build_if_stmt_node(Expr *cond, Stmt *then, Stmt *otherwise) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Error: Failed to build if statement node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    stmt->type                = STMT_IF;
    stmt->if_stmt.cond        = cond;
    stmt->if_stmt.then_branch = then;
    stmt->if_stmt.else_branch = otherwise;
    return stmt;
}

Stmt *ast_build_expr_stmt_node(Expr *expr) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (!stmt) {
        fprintf(stderr, "Error: Failed to build expression statement node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    stmt->type      = STMT_EXPR;
    stmt->expr_stmt = expr;
    return stmt;
}

Block *ast_build_block_node(Stmt **stmts, int cnt) {
    Block *block = malloc(sizeof(Block));
    if (!block) {
        fprintf(stderr, "Error: Failed to build block node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    block->stmts = stmts;
    block->cnt   = cnt;
    return block;
}

Function *ast_build_function_node(char *name, Block *body) {
    Function *function = malloc(sizeof(Function));
    if (!function) {
        fprintf(stderr, "Error: Failed to build function node. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    function->name  = name;
    function->body  = body;
    function->stack = 0;
    return function;
}

Program *ast_build_program_node(Function *function) {
    Program *program = malloc(sizeof(Program));
    if (!program) {
        fprintf(stderr, "Error: Failed to build program node. Out of memory.\n");
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