#pragma once

typedef struct {
    char *name;   /* Symbol name                  */
    int   offset; /* Symbol stack offset from rbp */
} Symbol;

typedef struct {
    Symbol **items; /* Current symbol table items    */
    int      len;   /* Current symbol table length   */
    int      cap;   /* Current symbol table capacity */
} SymbolTable;

/**
 * symbol_table_get - Finds the symbol associated with @name from @table.
 * @table: The table to find from.
 * @name: The symbol name.
 * Returns: The symbol found, or NULL if not found.
 */
Symbol *symbol_table_get(SymbolTable *table, char *name);

/**
 * symbol_table_add - Adds a symbol with fields @name and @offset into @table.
 * @table: The table to add into.
 * @name: The symbol name.
 * @offset: The symbol stack offset from rbp.
 * Returns: The symbol added.
 */
Symbol *symbol_table_add(SymbolTable *table, char *name, int offset);

/**
 * symbol_table_free - Frees the symbol table's allocated memory.
 * @table: The symbol table to free.
 */
void symbol_table_free(SymbolTable *table);

/**
 * symbol_table_init - Initializes @table.
 * @table: The symbol table to initialize.
 */
void symbol_table_init(SymbolTable *table);