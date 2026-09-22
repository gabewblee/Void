#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <symtbl.h>

VarSymbol *var_table_find(VarTable *table, const char *name) {
    for (int i = 0; i < table->len; i++) {
        if (!strcmp(table->items[i]->name, name))
            return table->items[i];
    }

    return NULL;
}

VarSymbol *var_table_add(VarTable *table, char *name, int offset) {
    if (table->len == table->cap) {
        int cap = table->cap << 1;

        VarSymbol **items = realloc(table->items, cap * sizeof(VarSymbol *));
        if (!items) {
            fprintf(stderr, "Error: Failed to build variable table. Out of memory.\n");
            exit(EXIT_FAILURE);
        }

        table->items = items;
        table->cap   = cap;
    }

    VarSymbol *symbol = malloc(sizeof(VarSymbol));
    if (!symbol) {
        fprintf(stderr, "Error: Failed to build variable. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol->name = name;
    if (!symbol->name) {
        fprintf(stderr, "Error: Failed to build variable. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol->offset = offset;
    table->items[table->len++] = symbol;
    return symbol;
}

void var_table_free(VarTable *table) {
    free(table->items);
}

void var_table_init(VarTable *table) {
    table->items = malloc(sizeof(VarSymbol *));
    if (!table->items) {
        fprintf(stderr, "Error: Failed to build variable table. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    table->len = 0;
    table->cap = 1;
}

FunctionSymbol *function_table_find(FunctionTable *table, const char *name) {
    for (int i = 0; i < table->len; i++) {
        if (!strcmp(table->items[i]->name, name))
            return table->items[i];
    }

    return NULL;
}

FunctionSymbol *function_table_add(FunctionTable *table, char *name, Function *function, int paramc) {
    if (table->len == table->cap) {
        int cap = table->cap << 1;
        FunctionSymbol **items = realloc(table->items, cap * sizeof(FunctionSymbol *));
        if (!items) {
            fprintf(stderr, "Error: Failed to build function table. Out of memory.\n");
            exit(EXIT_FAILURE);
        }

        table->items = items;
        table->cap   = cap;
    }

    FunctionSymbol *symbol = malloc(sizeof(FunctionSymbol));
    if (!symbol) {
        fprintf(stderr, "Error: Failed to build function symbol. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol->name = name;
    if (!symbol->name) {
        fprintf(stderr, "Error: Failed to build function symbol. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol->function = function;
    symbol->paramc   = paramc;
    table->items[table->len++] = symbol;
    return symbol;
}

void function_table_free(FunctionTable *table) {
    free(table->items);
}

void function_table_init(FunctionTable *table) {
    table->items = malloc(sizeof(FunctionSymbol *));
    if (!table->items) {
        fprintf(stderr, "Error: Failed to build function table. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    table->len = 0;
    table->cap = 1;
}
