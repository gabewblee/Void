#pragma once

typedef struct Function Function;

typedef struct {
    char *name;   /* Variable name                  */
    int   offset; /* Variable stack offset from rbp */
} VarSymbol;

typedef struct {
    VarSymbol **items; /* Current variable table items    */
    int         len;   /* Current variable table length   */
    int         cap;   /* Current variable table capacity */
} VarTable;

typedef struct {
    char     *name;     /* Function name            */
    Function *function; /* Function definition      */
    int       paramc;   /* Function parameter count */
} FunctionSymbol;

typedef struct {
    FunctionSymbol **items; /* Current function table items    */
    int              len;   /* Current function table length   */
    int              cap;   /* Current function table capacity */
} FunctionTable;

/**
 * var_table_find - Finds the variable associated with @name from @table.
 * @table: The table to find from.
 * @name: The variable name.
 * Returns: The variable found, or NULL if not found.
 */
VarSymbol *var_table_find(VarTable *table, const char *name);

/**
 * var_table_add - Adds a variable with fields @name and @offset into @table.
 * @table: The table to add into.
 * @name: The variable name.
 * @offset: The variable stack offset from rbp.
 * Returns: The variable added.
 */
VarSymbol *var_table_add(VarTable *table, char *name, int offset);

/**
 * var_table_free - Frees the variable table's allocated memory.
 * @table: The variable table to free.
 */
void var_table_free(VarTable *table);

/**
 * var_table_init - Initializes @table.
 * @table: The variable table to initialize.
 */
void var_table_init(VarTable *table);

/**
 * function_table_find - Finds the function associated with @name from @table.
 * @table: The table to find from.
 * @name: The function name.
 * Returns: The function found, or NULL if not found.
 */
FunctionSymbol *function_table_find(FunctionTable *table, const char *name);

/**
 * function_table_add - Adds a function with fields @name, @function and @paramc into @table.
 * @table: The table to add into.
 * @name: The function name.
 * @function: The function definition.
 * @paramc: The function parameter count.
 * Returns: The function added.
 */
FunctionSymbol *function_table_add(FunctionTable *table, char *name, Function *function, int paramc);

/**
 * function_table_free - Frees the function table's allocated memory.
 * @table: The function table to free.
 */
void function_table_free(FunctionTable *table);

/**
 * function_table_init - Initializes @table.
 * @table: The function table to initialize.
 */
void function_table_init(FunctionTable *table);
