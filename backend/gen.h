#pragma once

#include <parser.h>
#include <stdio.h>

/**
 * gen - Generates assembly program for @program in @out.
 * @out: The output file.
 * @program: The input program.
 */
void gen(FILE *out, Program *program);

/**
 * gen_free - Frees the generator's allocated memory.
 */
void gen_free();
