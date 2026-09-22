#pragma once

#include <symtbl.h>

typedef struct Scope Scope;
typedef struct Resolver Resolver;

struct Scope {
    VarTable variables; /* Scope's variable table */
    Scope   *parent;    /* Scope's parent          */
};

struct Resolver {
    FunctionTable functions;        /* Function table       */
    Scope        *scope;            /* Current scope        */
    int           nxt_stack_offset; /* Current stack offset */
};

/**
 * resolve - Resolves the program for defined variables.
 * @resolver: The resolver to define variables.
 * @program: The parsed program.
 */
void resolve(Resolver *resolver, Program *program);

/**
 * resolver_init - Initializes the resolver.
 * @resolver: The resolver to initialize.
 */
void resolver_init(Resolver *resolver);