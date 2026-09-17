#include <ast.h>
#include <resolver.h>
#include <stdio.h>
#include <stdlib.h>

static void scope_init(Resolver *resolver) {
    Scope *scope = malloc(sizeof(Scope));
    if (!scope) {
        fprintf(stderr, "Error: Failed to build scope. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol_table_init(&scope->symbols);
    scope->parent = resolver->scope;
    resolver->scope = scope;
}

static void scope_free(Resolver *resolver) {
    Scope *scope = resolver->scope;
    resolver->scope = resolver->scope->parent;
    symbol_table_free(&scope->symbols);
    free(scope);
}

static Symbol *resolve_symbol(Resolver *resolver, char *name) {
    for (Scope *scope = resolver->scope; scope; scope = scope->parent) {
        Symbol *symbol = symbol_table_get(&scope->symbols, name);
        if (symbol)
            return symbol;
    }

    return NULL;
}

static void resolve_expr(Resolver *resolver, Expr *expr) {
    switch (expr->type) {
    case EXPR_INTEGER:
        return;
    case EXPR_UNARY:
        resolve_expr(resolver, expr->unary.operand);
        return;
    case EXPR_BINARY:
        resolve_expr(resolver, expr->binary.left);
        resolve_expr(resolver, expr->binary.right);
        return;
    case EXPR_IDENTIFIER: {
        Symbol *symbol = resolve_symbol(resolver, expr->identifier.name);
        if (!symbol) {
            fprintf(stderr, "Error: Failed to resolve '%s'.\n", expr->identifier.name);
            exit(EXIT_FAILURE);
        }

        expr->identifier.symbol = symbol;
        return;
    }
    default:
        fprintf(stderr, "Error: Failed to resolve expression type '%d'.\n", expr->type);
        exit(EXIT_FAILURE);
    }
}

static void resolve_stmt(Resolver *resolver, Stmt *stmt) {
    switch (stmt->type) {
    case STMT_RETURN:
        resolve_expr(resolver, stmt->ret);
        return;
    case STMT_DECL: {
        if (symbol_table_get(&resolver->scope->symbols, stmt->decl.name)) {
            fprintf(stderr, "Error: Variable '%s' redefined.\n", stmt->decl.name);
            exit(EXIT_FAILURE);
        }

        stmt->decl.symbol = symbol_table_add(&resolver->scope->symbols, stmt->decl.name, resolver->nxt_stack_offset);
        resolver->nxt_stack_offset -= 4;
        if (stmt->decl.initializer)
            resolve_expr(resolver, stmt->decl.initializer);

        return;
    }
    case STMT_EXPR:
        resolve_expr(resolver, stmt->expr);
        return;
    default:
        fprintf(stderr, "Error: Failed to resolve statement type '%d'.\n", stmt->type);
        exit(EXIT_FAILURE);
    }
}

static void resolve_block(Resolver *resolver, Block *block) {
    scope_init(resolver);
    for (int i = 0; i < block->cnt; i++)
        resolve_stmt(resolver, block->stmts[i]);
}

static void resolve_function(Resolver *resolver, Function *function) {
    resolver->nxt_stack_offset = -4;
    resolve_block(resolver, function->body);
}

void resolve(Resolver *resolver, Program *program) {
    resolve_function(resolver, program->function);
}

void resolver_free(Resolver *resolver) {
    while (resolver->scope)
        scope_free(resolver);
}

void resolver_init(Resolver *resolver) {
    resolver->scope            = NULL;
    resolver->nxt_stack_offset = 0;
}