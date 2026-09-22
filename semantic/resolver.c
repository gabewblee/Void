#include <ast.h>
#include <resolver.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void resolve_block(Resolver *resolver, Block *block);

static void error(const char *fmt, ...) {
    fprintf(stderr, "error: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static void scope_init(Resolver *resolver) {
    Scope *scope = malloc(sizeof(Scope));
    if (!scope)
        error("out of memory");

    var_table_init(&scope->variables);
    scope->parent = resolver->scope;
    resolver->scope = scope;
}

static void scope_free(Resolver *resolver) {
    Scope *scope = resolver->scope;
    resolver->scope = resolver->scope->parent;
    var_table_free(&scope->variables);
    free(scope);
}

static VarSymbol *resolve_symbol(Resolver *resolver, char *name) {
    for (Scope *scope = resolver->scope; scope; scope = scope->parent) {
        VarSymbol *symbol = var_table_find(&scope->variables, name);
        if (symbol)
            return symbol;
    }

    return NULL;
}

static void resolve_expr(Resolver *resolver, Expr *expr) {
    switch (expr->type) {
    case EXPR_INT:
        return;
    case EXPR_UNARY:
        resolve_expr(resolver, expr->unary.operand);
        return;
    case EXPR_BINARY:
        resolve_expr(resolver, expr->binary.left);
        resolve_expr(resolver, expr->binary.right);
        return;
    case EXPR_ID: {
        VarSymbol *symbol = resolve_symbol(resolver, expr->id.name);
        if (!symbol)
            error("undeclared identifier '%s'", expr->id.name);

        expr->id.symbol = symbol;
        return;
    }
    case EXPR_ASSIGN: {
        switch (expr->assign.target->type) {
        case EXPR_ID:                                                   break;
        case EXPR_INT:    error("cannot assign to a number");           break;
        case EXPR_CALL:   error("cannot assign to a function call");    break;
        case EXPR_UNARY:  error("cannot assign to a unary expression"); break;
        case EXPR_BINARY: error("cannot assign to this expression");    break;
        case EXPR_ASSIGN: error("cannot assign to an assignment");      break;
        default:          error("cannot assign to this expression");    break;
        }

        resolve_expr(resolver, expr->assign.target);
        resolve_expr(resolver, expr->assign.val);
        return;
    }
    case EXPR_CALL: {
        FunctionSymbol *symbol = function_table_find(&resolver->functions, expr->call.name);
        if (!symbol)
            error("undeclared function '%s'", expr->call.name);

        if (expr->call.argc != symbol->paramc)
            error("'%s' expected %d arguments, got %d arguments", expr->call.name, symbol->paramc, expr->call.argc);
        
        for (int i = 0; i < expr->call.argc; i++)
            resolve_expr(resolver, expr->call.args[i]);
        
        expr->call.symbol = symbol;
        return;
    }
    default:
        error("internal error: unknown expression type %d", expr->type);
    }
}

static void resolve_stmt(Resolver *resolver, Stmt *stmt) {
    switch (stmt->type) {
    case STMT_RETURN:
        resolve_expr(resolver, stmt->ret_stmt);
        return;
    case STMT_DECL: {
        if (var_table_find(&resolver->scope->variables, stmt->decl_stmt.name))
            error("redefinition of '%s'", stmt->decl_stmt.name);

        stmt->decl_stmt.symbol = var_table_add(&resolver->scope->variables, stmt->decl_stmt.name, resolver->nxt_stack_offset);
        resolver->nxt_stack_offset -= 4;
        if (stmt->decl_stmt.initializer)
            resolve_expr(resolver, stmt->decl_stmt.initializer);

        return;
    }
    case STMT_IF:
        resolve_expr(resolver, stmt->if_stmt.cond);
        resolve_stmt(resolver, stmt->if_stmt.then_branch);
        if (stmt->if_stmt.else_branch)
            resolve_stmt(resolver, stmt->if_stmt.else_branch);
        return;
    case STMT_BLOCK:
        resolve_block(resolver, stmt->block_stmt);
        return;
    case STMT_WHILE:
        resolve_expr(resolver, stmt->while_stmt.cond);
        resolve_stmt(resolver, stmt->while_stmt.body);
        return;
    case STMT_FOR:
        resolve_stmt(resolver, stmt->for_stmt.init);
        resolve_expr(resolver, stmt->for_stmt.cond);
        resolve_expr(resolver, stmt->for_stmt.inc);
        resolve_stmt(resolver, stmt->for_stmt.body);
        return;
    case STMT_BREAK:
    case STMT_CONTINUE:
        return;
    case STMT_EXPR:
        resolve_expr(resolver, stmt->expr_stmt);
        return;
    default:
        error("internal error: unknown statement type %d", stmt->type);
    }
}

static void resolve_block(Resolver *resolver, Block *block) {
    scope_init(resolver);
    for (int i = 0; i < block->cnt; i++)
        resolve_stmt(resolver, block->stmts[i]);
    scope_free(resolver);
}

static void resolve_function_decl(Resolver *resolver, Function *function) {
    FunctionSymbol *entry = function_table_find(&resolver->functions, function->name);
    if (!entry) {
        function_table_add(&resolver->functions, function->name, function, function->paramc);
        return;
    }

    if (entry->paramc != function->paramc)
        error("conflicting declaration of '%s': expected %d parameters, got %d parameters", function->name, entry->paramc, function->paramc);

    if (function->body) {
        if (entry->function->body)
            error("redefinition of '%s'", function->name);

        entry->function = function;
    }
}

static void resolve_function_body(Resolver *resolver, Function *function) {
    if (!function->body)
        return;

    resolver->nxt_stack_offset = -4;
    scope_init(resolver);
    for (int i = 0; i < function->paramc; i++) {
        if (var_table_find(&resolver->scope->variables, function->params[i]))
            error("redefinition of parameter '%s'", function->params[i]);

        var_table_add(&resolver->scope->variables, function->params[i], resolver->nxt_stack_offset);
        resolver->nxt_stack_offset -= 4;
    }

    resolve_block(resolver, function->body);
    scope_free(resolver);
    function->stack = -resolver->nxt_stack_offset - 4;
}

void resolve(Resolver *resolver, Program *program) {
    for (int i = 0; i < program->functionc; i++)
        resolve_function_decl(resolver, program->functions[i]);

    for (int i = 0; i < program->functionc; i++)
        resolve_function_body(resolver, program->functions[i]);
}

void resolver_init(Resolver *resolver) {
    function_table_init(&resolver->functions);
    resolver->scope            = NULL;
    resolver->nxt_stack_offset = 0;
}