#pragma once

#include <symtbl.h>

typedef struct Scope Scope;
typedef struct Resolver Resolver;

struct Scope {
    SymbolTable symbols; /* Scope's symbol table */
    Scope      *parent;  /* Scope's parent       */
};

struct Resolver {
    Scope *scope;            /* Current scope        */
    int    nxt_stack_offset; /* Current stack offset */
};

/**
 * resolve - Resolves the program for defined variables.
 * @resolver: The resolver to define variables.
 * @program: The parsed program.
 */
void resolve(Resolver *resolver, Program *program);

/**
 * resolver_free - Frees the resolver's allocated memory.
 * @resolver: The resolver to free.
 */
void resolver_free(Resolver *resolver);

/**
 * resolver_init - Initializes the resolver.
 * @resolver: The resolver to initialize.
 */
void resolver_init(Resolver *resolver);