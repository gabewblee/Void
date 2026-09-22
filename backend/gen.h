#pragma once

#include <parser.h>
#include <stdio.h>
#include <symtbl.h>

/**
 * gen - Generates assembly program for @program in @out.
 * @out: The output file.
 * @program: The input program.
 * @symbols: The symbol table.
 */
void gen(FILE *out, Program *program, SymbolTable *symbols);

/**
 * gen_free - Frees the generator's allocated memory.
 */
void gen_free();
