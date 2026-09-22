#pragma once

#include <stddef.h>
#include <symbol.h>

typedef struct Function Function;

typedef enum {
    SYMBOL_VAR, /* Variable */
    SYMBOL_FUNC /* Function */
} SymbolKind;

typedef struct {
    int        offset;   /* Variable stack offset from rbp */
    char      *name;     /* Symbol name                    */
    SymbolKind kind;     /* Symbol kind                    */

    /* Function fields*/
    int        paramc;   /* Function parameter count       */
    Function  *function; /* Function definition            */
} Symbol;

typedef struct {
    Symbol *symbols; /* Symbol storage */
    size_t  symbolc; /* Symbol count   */
    size_t  cap;     /* Symbol capacity */
} SymbolTable;

/**
 * symbol_add - Adds @symbol to @table.
 * @table: The symbol table to add to.
 * @symbol: The symbol to add.
 * Returns: The symbol index.
 */
SymbolId symbol_add(SymbolTable *table, Symbol symbol);

/**
 * symbol_get - Gets the symbol @id from @table.
 * @table: The symbol table to get from.
 * @id: The symbol index.
 * Returns: The symbol @id.
 */
Symbol *symbol_get(SymbolTable *table, SymbolId id);

/**
* symbol_table_free - Frees @table.
* @table: The symbol table to free.
*/
void symbol_table_free(SymbolTable *table);

/**
 * symbol_table_init - Initializes @table.
 * @table: The symbol table to initialize.
 */
void symbol_table_init(SymbolTable *table);