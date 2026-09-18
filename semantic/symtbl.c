#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <symtbl.h>

Symbol *symbol_table_get(SymbolTable *table, char *name) {
    for (int i = 0; i < table->len; i++) {
        if (!strcmp(table->items[i]->name, name))
            return table->items[i];
    }

    return NULL;
}

Symbol *symbol_table_add(SymbolTable *table, char *name, int offset) {
    if (table->len == table->cap) {
        int cap = table->cap << 1;

        Symbol **items = realloc(table->items, cap * sizeof(Symbol *));
        if (!items) {
            fprintf(stderr, "Error: Failed to build symbol table. Out of memory.\n");
            exit(EXIT_FAILURE);
        }

        table->items = items;
        table->cap   = cap;
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    if (!symbol) {
        fprintf(stderr, "Error: Failed to build symbol. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol->name = name;
    if (!symbol->name) {
        fprintf(stderr, "Error: Failed to build symbol. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    symbol->offset = offset;
    table->items[table->len++] = symbol;
    return symbol;
}

void symbol_table_free(SymbolTable *table) {
    free(table->items);
}

void symbol_table_init(SymbolTable *table) {
    table->items = malloc(sizeof(Symbol *));
    if (!table->items) {
        fprintf(stderr, "Error: Failed to build symbol table. Out of memory.\n");
        exit(EXIT_FAILURE);
    }

    table->len = 0;
    table->cap = 1;
}