#include <stdlib.h>

#include "gen.h"

static void gen_expr(FILE *out, Expr *expr) {
    switch (expr->type) {
    case EXPR_INTEGER:
        fprintf(out, "    movl $%ld, %%eax\n", expr->integer);
        return;
    case EXPR_UNARY:
        gen_expr(out, expr->unary.operand);
        switch (expr->unary.op) {
        case TOKEN_PLUS:
            return;
        case TOKEN_MINUS:
            fprintf(out, "    negl %%eax\n");
            return;
        default:
            break;
        }
        break;
    case EXPR_BINARY:
        /*
         *     gen_expr(out, expr->binary.left)
         *     push %rax
         *
         *     gen_expr(out, expr->binary.right)
         *     popq %rcx
         */
        gen_expr(out, expr->binary.left);
        fprintf(out, "    pushq %%rax\n");

        gen_expr(out, expr->binary.right);
        fprintf(out, "    popq %%rcx\n");

        switch (expr->binary.op) {
        case TOKEN_PLUS:
            /*
             *    addl %ecx, %eax
             */
            fprintf(out, "    addl %%ecx, %%eax\n");
            return;
        case TOKEN_MINUS:
            /*
             *    subl %eax, %ecx
             *    movl %ecx, %eax
             */
            fprintf(out, "    subl %%eax, %%ecx\n");
            fprintf(out, "    movl %%ecx, %%eax\n");
            return;
        case TOKEN_STAR:
            /*
             * imull %ecx, %eax
             */
            fprintf(out, "    imull %%ecx, %%eax\n");
            return;
        case TOKEN_SLASH:
            /*
             *     movl %eax, %esi
             *     movl %ecx, %eax
             *     cltd
             *     idivl %esi
             */
            fprintf(out, "    movl %%eax, %%esi\n");
            fprintf(out, "    movl %%ecx, %%eax\n");
            fprintf(out, "    cltd\n");
            fprintf(out, "    idivl %%esi\n");
            return;
        default:
            break;
        }
        break;
    }

    fprintf(stderr, "Error: Unsupported expression\n");
    exit(EXIT_FAILURE);
}

static void gen_return_stmt(FILE *out, ReturnStmt *stmt) {
    /*
     * gen_expr(out, stmt->expr)
     *   ret
     */
    gen_expr(out, stmt->expr);
    fprintf(out, "    ret\n");
}

static void gen_function(FILE *out, Function *function) {
    /*
     * .global function->name
     * function->name:
     * gen_return_stmt(out, function->body)
     */
    fprintf(out, ".global %s\n", function->name);
    fprintf(out, "%s:\n", function->name);
    gen_return_stmt(out, function->body);
}

void gen(FILE *out, Program *program) {
    /*
     * .text
     * gen_function(out, program->function)
     */
    fprintf(out, ".text\n");
    gen_function(out, program->function);
}
