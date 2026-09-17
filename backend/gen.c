#include <gen.h>
#include <stdlib.h>

static void gen_expr(FILE *out, Expr *expr) {
    switch (expr->type) {
    case EXPR_INTEGER:
        fprintf(out, "    mov eax, %ld\n", expr->integer);
        return;
    case EXPR_UNARY:
        gen_expr(out, expr->unary.operand);
        switch (expr->unary.op) {
        case TOKEN_PLUS:
            return;
        case TOKEN_MINUS:
            fprintf(out, "    neg eax\n");
            return;
        default:
            break;
        }
        break;
    case EXPR_BINARY:
        /*
         * gen_expr(out, expr->binary.left)
         *     push rax
         *
         * gen_expr(out, expr->binary.right)
         *     pop rcx
         */
        gen_expr(out, expr->binary.left);
        fprintf(out, "    push rax\n");

        gen_expr(out, expr->binary.right);
        fprintf(out, "    pop rcx\n");

        switch (expr->binary.op) {
        case TOKEN_PLUS:
            /*
             *    add eax, ecx
             */
            fprintf(out, "    add eax, ecx\n");
            return;
        case TOKEN_MINUS:
            /*
             *    sub ecx, eax
             *    mov eax, ecx
             */
            fprintf(out, "    sub ecx, eax\n");
            fprintf(out, "    mov eax, ecx\n");
            return;
        case TOKEN_STAR:
            /*
             *      imul eax, ecx
             */
            fprintf(out, "    imul eax, ecx\n");
            return;
        case TOKEN_SLASH:
            /*
             *     mov esi, eax
             *     mov eax, ecx
             *     cdq
             *     idiv esi
             */
            fprintf(out, "    mov esi, eax\n");
            fprintf(out, "    mov eax, ecx\n");
            fprintf(out, "    cdq\n");
            fprintf(out, "    idiv esi\n");
            return;
        default:
            break;
        }
        break;
    case EXPR_IDENTIFIER:
        fprintf(out, "    mov eax, dword [rbp%+d]\n", expr->identifier.symbol->offset);
        return;
    }

    fprintf(stderr, "Error: Unsupported expression\n");
    exit(EXIT_FAILURE);
}

static void gen_return_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->expr)
     *     ret
     */
    gen_expr(out, stmt->ret);
    fprintf(out, "    jmp .return\n");
}

static void gen_decl_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->decl.initializer)
     *     mov dword [rbp+stmt->decl.symbol->offset], eax
     */
    if (stmt->decl.initializer) {
        gen_expr(out, stmt->decl.initializer);
        fprintf(out, "     mov dword [rbp%+d], eax\n", stmt->decl.symbol->offset);
    }
}

static void gen_expr_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->expr)
     */
    gen_expr(out, stmt->expr);
}

static void gen_stmt(FILE *out, Stmt *stmt) {
    switch (stmt->type) {
    case STMT_RETURN:
        gen_return_stmt(out, stmt);
        return;
    case STMT_DECL:
        gen_decl_stmt(out, stmt);
        return;
    case STMT_EXPR:
        gen_expr_stmt(out, stmt);
        return;
    default:
        fprintf(stderr, "Error: Failed to recognize statement type '%d'.\n", stmt->type);
        exit(EXIT_FAILURE);
    }
}

static void gen_block(FILE *out, Block *block) {
    for (int i = 0; i < block->cnt; i++)
        gen_stmt(out, block->stmts[i]);
}

static void gen_function(FILE *out, Function *function) {
    /*
     * global function->name
     * function->name:
     *     push rbp
     *     mov rbp, rsp
     * gen_block(out, function->body)
     * .return:
     *     mov rsp, rbp
     *     pop rbp
     *     ret
     */
    fprintf(out, "global %s\n", function->name);
    fprintf(out, "%s:\n", function->name);
    fprintf(out, "    push rbp\n");
    fprintf(out, "    mov rbp, rsp\n");
    gen_block(out, function->body);
    fprintf(out, ".return:\n");
    fprintf(out, "    mov rsp, rbp\n");
    fprintf(out, "    pop rbp\n");
    fprintf(out, "    ret\n");
}

void gen(FILE *out, Program *program) {
    /*
     * section .text
     * gen_function(out, program->function)
     */
    fprintf(out, "section .text\n");
    gen_function(out, program->function);
}