#pragma once

#include <symtbl.h>

typedef struct Scope    Scope;
typedef struct Resolver Resolver;

typedef struct {
    char    *name; /* Bound identifier */
    SymbolId id;   /* Symbol for @name */
} Binding;

struct Scope {
    Binding *bindings; /* Names bound in this scope */
    int      len;      /* Binding count             */
    int      cap;      /* Binding capacity          */
    Scope   *parent;   /* Enclosing scope           */
};

struct Resolver {
    SymbolTable symbols;   /* Owns every symbol       */
    Scope       functions; /* Function name bindings  */
    Scope      *scope;     /* Current variable scope  */
    int         offset;    /* Current stack offset    */
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

/**
 * resolver_free - Frees the resolver's allocated memory.
 * @resolver: The resolver to free.
 */
void resolver_free(Resolver *resolver);
