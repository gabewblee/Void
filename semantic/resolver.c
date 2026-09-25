#include <ast.h>
#include <resolver.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    scope->bindings = NULL;
    scope->len      = 0;
    scope->cap      = 0;
    scope->parent   = resolver->scope;
    resolver->scope = scope;
}

static void scope_free(Resolver *resolver) {
    Scope *scope = resolver->scope;
    resolver->scope = scope->parent;
    free(scope->bindings);
    free(scope);
}

static void scope_bind(Scope *scope, char *name, SymbolId id) {
    if (scope->len == scope->cap) {
        int cap = scope->cap ? scope->cap << 1 : 1;
        Binding *bindings = realloc(scope->bindings, cap * sizeof(Binding));
        if (!bindings)
            error("out of memory");

        scope->bindings = bindings;
        scope->cap      = cap;
    }

    scope->bindings[scope->len].name = name;
    scope->bindings[scope->len].id   = id;
    scope->len++;
}

static SymbolId scope_find(Scope *scope, const char *name) {
    for (int i = 0; i < scope->len; i++) {
        if (!strcmp(scope->bindings[i].name, name))
            return scope->bindings[i].id;
    }

    return SYMBOL_INVALID;
}

static SymbolId resolve_symbol(Resolver *resolver, char *name) {
    for (Scope *scope = resolver->scope; scope; scope = scope->parent) {
        SymbolId id = scope_find(scope, name);
        if (id != SYMBOL_INVALID)
            return id;
    }

    return SYMBOL_INVALID;
}

static void resolve_expr(Resolver *resolver, Expr *expr) {
    switch (expr->kind) {
    case EXPR_INT:
        return;
    case EXPR_UNARY:
        resolve_expr(resolver, expr->unary_expr.operand);
        return;
    case EXPR_BINARY:
        resolve_expr(resolver, expr->binary_expr.left);
        resolve_expr(resolver, expr->binary_expr.right);
        return;
    case EXPR_ID: {
        SymbolId id = resolve_symbol(resolver, expr->id_expr.name);
        if (id == SYMBOL_INVALID)
            error("undeclared identifier '%s'", expr->id_expr.name);

        expr->id_expr.symbol = id;
        return;
    }
    case EXPR_ASSIGN: {
        switch (expr->assign_expr.target->kind) {
        case EXPR_ID:                                                   break;
        case EXPR_INT:    error("cannot assign to a number");           break;
        case EXPR_CALL:   error("cannot assign to a function call");    break;
        case EXPR_UNARY:  error("cannot assign to a unary expression"); break;
        case EXPR_BINARY: error("cannot assign to this expression");    break;
        case EXPR_ASSIGN: error("cannot assign to an assignment");      break;
        default:          error("cannot assign to this expression");    break;
        }

        resolve_expr(resolver, expr->assign_expr.target);
        resolve_expr(resolver, expr->assign_expr.val);
        return;
    }
    case EXPR_CALL: {
        SymbolId id = scope_find(&resolver->functions, expr->call_expr.name);
        if (id == SYMBOL_INVALID)
            error("undeclared function '%s'", expr->call_expr.name);

        int paramc = symbol_get(&resolver->symbols, id)->paramc;
        if (expr->call_expr.argc != paramc)
            error("'%s' expected %d arguments, got %d arguments", expr->call_expr.name, paramc, expr->call_expr.argc);

        for (int i = 0; i < expr->call_expr.argc; i++)
            resolve_expr(resolver, expr->call_expr.args[i]);

        expr->call_expr.symbol = id;
        return;
    }
    default:
        error("internal error: unknown expression kind %d", expr->kind);
    }
}

static void resolve_stmt(Resolver *resolver, Stmt *stmt) {
    switch (stmt->kind) {
    case STMT_RETURN:
        resolve_expr(resolver, stmt->ret_stmt);
        return;
    case STMT_DECL: {
        if (scope_find(resolver->scope, stmt->decl_stmt.name) != SYMBOL_INVALID)
            error("redefinition of '%s'", stmt->decl_stmt.name);

        Symbol symbol = {
            .name   = stmt->decl_stmt.name,
            .kind   = SYMBOL_VAR,
            .offset = resolver->offset,
        };
        SymbolId id = symbol_add(&resolver->symbols, symbol);
        scope_bind(resolver->scope, stmt->decl_stmt.name, id);
        stmt->decl_stmt.symbol = id;
        resolver->offset -= 4;
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
        error("internal error: unknown statement kind %d", stmt->kind);
    }
}

static void resolve_block(Resolver *resolver, Block *block) {
    scope_init(resolver);
    for (int i = 0; i < block->stmtc; i++)
        resolve_stmt(resolver, block->stmts[i]);
    scope_free(resolver);
}

static void resolve_function_decl(Resolver *resolver, Function *function) {
    SymbolId id = scope_find(&resolver->functions, function->name);
    if (id == SYMBOL_INVALID) {
        Symbol symbol = {
            .name     = function->name,
            .kind     = SYMBOL_FUNC,
            .function = function,
            .paramc   = function->paramc,
        };
        id = symbol_add(&resolver->symbols, symbol);
        scope_bind(&resolver->functions, function->name, id);
        return;
    }

    Symbol *entry = symbol_get(&resolver->symbols, id);
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

    resolver->offset = -4;
    scope_init(resolver);
    for (int i = 0; i < function->paramc; i++) {
        if (scope_find(resolver->scope, function->params[i]) != SYMBOL_INVALID)
            error("redefinition of parameter '%s'", function->params[i]);

        Symbol symbol = {
            .name   = function->params[i],
            .kind   = SYMBOL_VAR,
            .offset = resolver->offset,
        };
        SymbolId id = symbol_add(&resolver->symbols, symbol);
        scope_bind(resolver->scope, function->params[i], id);
        resolver->offset -= 4;
    }

    resolve_block(resolver, function->body);
    scope_free(resolver);
    function->stack = -resolver->offset - 4;
}

void resolve(Resolver *resolver, Program *program) {
    for (int i = 0; i < program->functionc; i++)
        resolve_function_decl(resolver, program->functions[i]);

    for (int i = 0; i < program->functionc; i++)
        resolve_function_body(resolver, program->functions[i]);
}

void resolver_init(Resolver *resolver) {
    symbol_table_init(&resolver->symbols);
    resolver->functions.bindings = NULL;
    resolver->functions.len      = 0;
    resolver->functions.cap      = 0;
    resolver->functions.parent   = NULL;
    resolver->scope              = NULL;
    resolver->offset             = 0;
}

void resolver_free(Resolver *resolver) {
    symbol_table_free(&resolver->symbols);
    free(resolver->functions.bindings);
}
