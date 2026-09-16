#include "gen.h"

static void gen_expr(FILE *out, Expr *expr) {
    /*
     *   mov $expr->integer, %eax
     */
    fprintf(out, "  mov $%ld, %%eax\n", expr->integer);
}

static void gen_return_stmt(FILE *out, ReturnStmt *stmt) {
    /*
     * gen_expr()
     *   ret
     */
    gen_expr(out, stmt->expr);
    fprintf(out, "  ret\n");
}

static void gen_function(FILE *out, Function *function) {
    /*
     * .global function->name
     * function->name:
     * gen_return_stmt()
     */
    fprintf(out, ".global %s\n", function->name);
    fprintf(out, "%s:\n", function->name);
    gen_return_stmt(out, function->body);
}

void gen(FILE *out, Program *program) {
    /*
     * .text
     * gen_function()
     */
    fprintf(out, ".text\n");
    gen_function(out, program->function);
}
