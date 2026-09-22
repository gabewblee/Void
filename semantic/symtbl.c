#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <symtbl.h>

SymbolId symbol_add(SymbolTable *table, Symbol symbol) {
    if (table->symbolc == table->cap) {
        size_t cap = table->cap ? table->cap << 1 : 1;
        Symbol *symbols = realloc(table->symbols, cap * sizeof(Symbol));
        if (!symbols) {
            fprintf(stderr, "error: out of memory\n");
            exit(EXIT_FAILURE);
        }

        table->symbols = symbols;
        table->cap     = cap;
    }

    if (table->symbolc >= UINT32_MAX) {
        fprintf(stderr, "error: out of symbols\n");
        exit(EXIT_FAILURE);
    }

    SymbolId id = (SymbolId)table->symbolc++;
    table->symbols[id] = symbol;
    return id;
}

Symbol *symbol_get(SymbolTable *table, SymbolId id) {
    assert(id < table->symbolc);
    return &table->symbols[id];
}

void symbol_table_free(SymbolTable *table) {
    free(table->symbols);
}

void symbol_table_init(SymbolTable *table) {
    table->symbols = NULL;
    table->symbolc = 0;
    table->cap     = 0;
}