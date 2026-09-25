#include <ast.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void ast_free_stmt_node(Stmt *stmt);
static void ast_free_block_node(Block *block);

static void error(char *fmt, ...) {
    fprintf(stderr, "error: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static Expr *ast_build_expr_node(ExprKind kind) {
    Expr *expr = malloc(sizeof(Expr));
    if (!expr)
        error("out of memory");

    expr->kind = kind;
    return expr;
}

static Stmt *ast_build_stmt_node(StmtKind kind) {
    Stmt *stmt = malloc(sizeof(Stmt));
    if (!stmt)
        error("out of memory");

    stmt->kind = kind;
    return stmt;
}

static void ast_free_name(char *s) {
    if (!s)
        return;

    free(s);
}

static void ast_free_expr_node(Expr *expr) {
    if (!expr)
        return;

    switch (expr->kind) {
        case EXPR_INT:
            break;
        case EXPR_UNARY:
            ast_free_expr_node(expr->unary_expr.operand);
            break;
        case EXPR_BINARY:
            ast_free_expr_node(expr->binary_expr.left);
            ast_free_expr_node(expr->binary_expr.right);
            break;
        case EXPR_ID:
            ast_free_name(expr->id_expr.name);
            break;
        case EXPR_ASSIGN:
            ast_free_expr_node(expr->assign_expr.target);
            ast_free_expr_node(expr->assign_expr.val);
            break;
        case EXPR_CALL:
            ast_free_name(expr->call_expr.name);
            for (int i = 0; i < expr->call_expr.argc; i++)
                ast_free_expr_node(expr->call_expr.args[i]);
            free(expr->call_expr.args);
            break;
        default:
            error("internal error: unknown expression kind %d", expr->kind);
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
}

static void ast_free_if_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->if_stmt.cond);
    ast_free_stmt_node(stmt->if_stmt.then_branch);
    ast_free_stmt_node(stmt->if_stmt.else_branch);
}

static void ast_free_while_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->while_stmt.cond);
    ast_free_stmt_node(stmt->while_stmt.body);
}

static void ast_free_for_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_stmt_node(stmt->for_stmt.init);
    ast_free_expr_node(stmt->for_stmt.cond);
    ast_free_expr_node(stmt->for_stmt.inc);
    ast_free_stmt_node(stmt->for_stmt.body);
}

static void ast_free_expr_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    ast_free_expr_node(stmt->expr_stmt);
}

static void ast_free_stmt_node(Stmt *stmt) {
    if (!stmt)
        return;

    switch (stmt->kind) {
    case STMT_RETURN:
        ast_free_ret_stmt_node(stmt);
        break;
    case STMT_DECL:
        ast_free_decl_stmt_node(stmt);
        break;
    case STMT_IF:
        ast_free_if_stmt_node(stmt);
        break;
    case STMT_BLOCK:
        ast_free_block_node(stmt->block_stmt);
        break;
    case STMT_WHILE:
        ast_free_while_stmt_node(stmt);
        break;
    case STMT_FOR:
        ast_free_for_stmt_node(stmt);
        break;
    case STMT_EXPR:
        ast_free_expr_stmt_node(stmt);
        break;
    case STMT_BREAK:
    case STMT_CONTINUE:
        break;
    default:
        error("internal error: unknown statement kind %d", stmt->kind);
    }
    
    free(stmt);
}

static void ast_free_block_node(Block *block) {
    if (!block)
        return;

    for (int i = 0; i < block->stmtc; i++)
        ast_free_stmt_node(block->stmts[i]);

    free(block->stmts);
    free(block);
}

static void ast_free_function_node(Function *function) {
    if (!function)
        return;

    free(function->name);
    for (int i = 0; i < function->paramc; i++)
        free(function->params[i]);
    
    free(function->params);
    ast_free_block_node(function->body);
    free(function);
}

Expr *ast_build_integer_expr_node(long integer) {
    Expr *expr = ast_build_expr_node(EXPR_INT);
    expr->int_expr = integer;
    return expr;
}

Expr *ast_build_unary_expr_node(TokenKind op, Expr *operand) {
    Expr *expr = ast_build_expr_node(EXPR_UNARY);
    expr->unary_expr.op      = op;
    expr->unary_expr.operand = operand;
    return expr;
}

Expr *ast_build_binary_expr_node(TokenKind op, Expr *left, Expr *right) {
    Expr *expr = ast_build_expr_node(EXPR_BINARY);
    expr->binary_expr.op    = op;
    expr->binary_expr.left  = left;
    expr->binary_expr.right = right;
    return expr;
}

Expr *ast_build_id_expr_node(char *name) {
    Expr *expr = ast_build_expr_node(EXPR_ID);
    expr->id_expr.name   = name;
    expr->id_expr.symbol = SYMBOL_INVALID;
    return expr;
}

Expr *ast_build_assign_expr_node(Expr *target, Expr *val) {
    Expr *expr = ast_build_expr_node(EXPR_ASSIGN);
    expr->assign_expr.target = target;
    expr->assign_expr.val    = val;
    return expr;
}

Expr *ast_build_call_expr_node(char *name, Expr **args, int argc) {
    Expr *expr = ast_build_expr_node(EXPR_CALL);
    expr->call_expr.name   = name;
    expr->call_expr.args   = args;
    expr->call_expr.argc   = argc;
    expr->call_expr.symbol = SYMBOL_INVALID;
    return expr;
}

Stmt *ast_build_ret_stmt_node(Expr *ret_expr) {
    Stmt *stmt = ast_build_stmt_node(STMT_RETURN);
    stmt->ret_stmt = ret_expr;
    return stmt;
}

Stmt *ast_build_decl_stmt_node(char *name, Expr *initializer) {
    Stmt *stmt = ast_build_stmt_node(STMT_DECL);
    stmt->decl_stmt.name        = name;
    stmt->decl_stmt.initializer = initializer;
    stmt->decl_stmt.symbol      = SYMBOL_INVALID;
    return stmt;
}

Stmt *ast_build_if_stmt_node(Expr *cond, Stmt *then, Stmt *otherwise) {
    Stmt *stmt = ast_build_stmt_node(STMT_IF);
    stmt->if_stmt.cond        = cond;
    stmt->if_stmt.then_branch = then;
    stmt->if_stmt.else_branch = otherwise;
    return stmt;
}

Stmt *ast_build_block_stmt_node(Block *block) {
    Stmt *stmt = ast_build_stmt_node(STMT_BLOCK);
    stmt->block_stmt = block;
    return stmt;
}

Stmt *ast_build_while_stmt_node(Expr *cond, Stmt *body) {
    Stmt *stmt = ast_build_stmt_node(STMT_WHILE);
    stmt->while_stmt.cond = cond;
    stmt->while_stmt.body = body;
    return stmt;
}

Stmt *ast_build_for_stmt_node(Stmt *init, Expr *cond, Expr *inc, Stmt *body) {
    Stmt *stmt = ast_build_stmt_node(STMT_FOR);
    stmt->for_stmt.init = init;
    stmt->for_stmt.cond = cond;
    stmt->for_stmt.inc  = inc;
    stmt->for_stmt.body = body;
    return stmt;
}

Stmt *ast_build_break_stmt_node() {
    Stmt *stmt = ast_build_stmt_node(STMT_BREAK);
    return stmt;
}

Stmt *ast_build_continue_stmt_node() {
    Stmt *stmt = ast_build_stmt_node(STMT_CONTINUE);
    return stmt;
}

Stmt *ast_build_expr_stmt_node(Expr *expr) {
    Stmt *stmt = ast_build_stmt_node(STMT_EXPR);
    stmt->expr_stmt = expr;
    return stmt;
}

Block *ast_build_block_node(Stmt **stmts, int stmtc) {
    Block *block = malloc(sizeof(Block));
    if (!block)
        error("out of memory");

    block->stmts = stmts;
    block->stmtc = stmtc;
    return block;
}

Function *ast_build_function_node(char *name, char **params, int paramc, Block *body) {
    Function *function = malloc(sizeof(Function));
    if (!function)
        error("out of memory");

    function->name   = name;
    function->params = params;
    function->paramc = paramc;
    function->body   = body;
    function->stack  = 0;
    return function;
}

Program *ast_build_program_node(Function **functions, int functionc) {
    Program *program = malloc(sizeof(Program));
    if (!program)
        error("out of memory");

    program->functions = functions;
    program->functionc = functionc;
    return program;
}

void ast_free_program_node(Program *program) {
    if (!program)
        return;

    for (int i = 0; i < program->functionc; i++)
        ast_free_function_node(program->functions[i]);
    
    free(program->functions);
    free(program);
}