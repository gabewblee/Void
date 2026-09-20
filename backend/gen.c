#include <gen.h>
#include <stdlib.h>

static int label = 0;

static void gen_stmt(FILE *out, Stmt *stmt);

static int gen_label() {
    return label++;
}

static void gen_expr(FILE *out, Expr *expr) {
    switch (expr->type) {
    case EXPR_INT:
        /*
         *     mov eax
         */
        fprintf(out, "    mov eax, %ld\n", expr->integer);
        return;
    case EXPR_UNARY:
        /*
         * gen_expr(out, expr->unary.operand)
         */
        gen_expr(out, expr->unary.operand);
        switch (expr->unary.op) {
        case TOKEN_PLUS:
            return;
        case TOKEN_MINUS:
            /*
             *     neg eax
             */
            fprintf(out, "    neg eax\n");
            return;
        case TOKEN_NOT:
            /*
             *     test eax, eax # Zero flag = 1 if eax == 0
             *     sete al       # al = 1 if zero flag == 1
             *     movzx eax, al # Zero extend eax with al as its lower byte
             */
            fprintf(out, "    test eax, eax\n");
            fprintf(out, "    sete al\n");
            fprintf(out, "    movzx eax, al\n");
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
        case TOKEN_MULT:
            /*
             *      imul eax, ecx
             */
            fprintf(out, "    imul eax, ecx\n");
            return;
        case TOKEN_DIV:
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
        case TOKEN_LESS:
            /*
             *     cmp ecx, eax
             *     setl al
             *     movzx eax, al
             */
            fprintf(out, "    cmp ecx, eax\n");
            fprintf(out, "    setl al\n");
            fprintf(out, "    movzx eax, al\n");
            return;
        case TOKEN_LEQ:
            /*
             *     cmp ecx, eax
             *     setle al
             *     movzx eax, al
             */
            fprintf(out, "    cmp ecx, eax\n");
            fprintf(out, "    setle al\n");
            fprintf(out, "    movzx eax, al\n");
            return;
        case TOKEN_EQEQ:
            /*
             *     cmp ecx, eax
             *     sete al
             *     movzx eax, al
             */
            fprintf(out, "    cmp ecx, eax\n");
            fprintf(out, "    sete al\n");
            fprintf(out, "    movzx eax, al\n");
            return;
        case TOKEN_GEQ:
            /*
             *     cmp ecx, eax
             *     setge al
             *     movzx eax, al
             */
            fprintf(out, "    cmp ecx, eax\n");
            fprintf(out, "    setge al\n");
            fprintf(out, "    movzx eax, al\n");
            return;
        case TOKEN_GREATER:
            /*
             *     cmp ecx, eax
             *     setg al
             *     movzx eax, al
             */
            fprintf(out, "    cmp ecx, eax\n");
            fprintf(out, "    setg al\n");
            fprintf(out, "    movzx eax, al\n");
            return;
        case TOKEN_NEQ:
            /*
             *     cmp eax, ecx
             *     setne al
             *     movzx eax, al
             */
            fprintf(out, "    cmp ecx, eax\n");
            fprintf(out, "    setne al\n");
            fprintf(out, "    movzx eax, al\n");
            return;
        default:
            break;
        }
        break;
    case EXPR_ID:
        /*
         *     mov eax, dword [rbp+expr->id.symbol->offset]
         */
        fprintf(out, "    mov eax, dword [rbp%+d]\n", expr->id.symbol->offset);
        return;
    case EXPR_ASSIGN:
        /*
         * gen_expr(out, expr->assign.val)
         *     mov dword [rbp+expr->assign.target->id.symbol->offset], eax
         */
        gen_expr(out, expr->assign.val);
        fprintf(out, "    mov dword [rbp%+d], eax\n", expr->assign.target->id.symbol->offset);

        return;
    }

    fprintf(stderr, "Error: Unsupported expression\n");
    exit(EXIT_FAILURE);
}

static void gen_return_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->ret_stmt)
     *     jmp .return
     */
    gen_expr(out, stmt->ret_stmt);
    fprintf(out, "    jmp .return\n");
}

static void gen_decl_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->decl_stmt.initializer)
     *     mov dword [rbp+stmt->decl_stmt.symbol->offset], eax
     */
    if (stmt->decl_stmt.initializer) {
        gen_expr(out, stmt->decl_stmt.initializer);
        fprintf(out, "    mov dword [rbp%+d], eax\n", stmt->decl_stmt.symbol->offset);
    }
}

static void gen_if_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->if_stmt.cond)
     *     test eax, eax
     * 
     * if (stmt->if_stmt.else_branch) {
     *         je .Lelselabel
     *     gen_stmt(out, stmt->if_stmt.then_branch)
     *         jmp .Ldonelabel
     *     .Lelselabel:
     *     gen_stmt(out, stmt->if_stmt.else_branch)
     *     .Ldonelabel:
     * } else {
     *         je .donelabel
     *     gen_stmt(out, stmt->if_stmt.then_branch)
     *         .Ldonelabel:
     * }
     */
    int label = gen_label();
    gen_expr(out, stmt->if_stmt.cond);
    fprintf(out, "    test eax, eax\n");
    if (stmt->if_stmt.else_branch) {
        fprintf(out, "    je .Lelse%d\n", label);
        gen_stmt(out, stmt->if_stmt.then_branch);
        fprintf(out, "    jmp .Ldone%d\n", label);
        fprintf(out, ".Lelse%d:\n", label);
        gen_stmt(out, stmt->if_stmt.else_branch);
        fprintf(out, ".Ldone%d:\n", label);
    } else {
        fprintf(out, "    je .Ldone%d\n", label);
        gen_stmt(out, stmt->if_stmt.then_branch);
        fprintf(out, ".Ldone%d:\n", label);
    }
}

static void gen_expr_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->expr_stmt)
     */
    gen_expr(out, stmt->expr_stmt);
}

static void gen_stmt(FILE *out, Stmt *stmt) {
    switch (stmt->type) {
    case STMT_RETURN:
        gen_return_stmt(out, stmt);
        return;
    case STMT_DECL:
        gen_decl_stmt(out, stmt);
        return;
    case STMT_IF:
        gen_if_stmt(out, stmt);
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
    fprintf(out, "    sub rsp, %d\n", function->stack);
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