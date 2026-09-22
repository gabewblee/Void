#include <gen.h>
#include <stdarg.h>
#include <stdlib.h>

typedef struct LoopCtx LoopCtx;

struct LoopCtx {
    int      label;
    LoopCtx *parent;
};

static int          label   = 0;
static LoopCtx     *ctx     = NULL;
static int          stack   = 0;
static SymbolTable *symbols = NULL;

static void gen_stmt(FILE *out, Stmt *stmt);
static void gen_block(FILE *out, Block *block);

static void error(const char *fmt, ...) {
    fprintf(stderr, "error: ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

static int gen_label() {
    return label++;
}

/* The invariant for expressions is that results are placed in rax */
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
            error("internal error: unsupported unary operator %d", expr->unary.op);
        }
        return;
    case EXPR_BINARY:
        switch (expr->binary.op) {
        case TOKEN_ANDAND: {
            /*
            * gen_expr(out, expr->binary.left)
            *     test eax, eax
            *     je .Lfalse
            * gen_expr(out, expr->binary.right)
            *     test eax, eax
            *     je .Lfalse
            *     mov eax, 1
            *     jmp .Ldone
            * .Lfalselabel:
            *     mov eax, 0
            * .Ldonelabel:
            */
            int label = gen_label();
            gen_expr(out, expr->binary.left);
            fprintf(out, "    test eax, eax\n");
            fprintf(out, "    je .Lfalse%d\n", label);
            gen_expr(out, expr->binary.right);
            fprintf(out, "    test eax, eax\n");
            fprintf(out, "    je .Lfalse%d\n", label);
            fprintf(out, "    mov eax, 1\n");
            fprintf(out, "    jmp .Ldone%d\n", label);
            fprintf(out, ".Lfalse%d:\n", label);
            fprintf(out, "    mov eax, 0\n");
            fprintf(out, ".Ldone%d:\n", label);
            return;
        } case TOKEN_OROR: {
            /*
            * gen_expr(out, expr->binary.left)
            *     test eax, eax
            *     jne .Ltrue
            * gen_expr(out, expr->binary.right)
            *     test eax, eax
            *     jne .Ltrue
            *     mov eax, 0
            *     jmp .Ldone
            * .Ltruelabel:
            *     mov eax, 1
            * .Ldonelabel:
            */
            int label = gen_label();
            gen_expr(out, expr->binary.left);
            fprintf(out, "    test eax, eax\n");
            fprintf(out, "    jne .Ltrue%d\n", label);
            gen_expr(out, expr->binary.right);
            fprintf(out, "    test eax, eax\n");
            fprintf(out, "    jne .Ltrue%d\n", label);
            fprintf(out, "    mov eax, 0\n");
            fprintf(out, "    jmp .Ldone%d\n", label);
            fprintf(out, ".Ltrue%d:\n", label);
            fprintf(out, "    mov eax, 1\n");
            fprintf(out, ".Ldone%d:\n", label);
            return;
        } default:
            break;
        }

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
            error("internal error: unsupported binary operator %d", expr->binary.op);
        }
        return;
    case EXPR_ID:
        /*
         *     mov eax, dword [rbp+offset]
         */
        fprintf(out, "    mov eax, dword [rbp%+d]\n", symbol_get(symbols, expr->id.symbol)->offset);
        return;
    case EXPR_ASSIGN:
        /*
         * gen_expr(out, expr->assign.val)
         *     mov dword [rbp+offset], eax
         */
        gen_expr(out, expr->assign.val);
        fprintf(out, "    mov dword [rbp%+d], eax\n", symbol_get(symbols, expr->assign.target->id.symbol)->offset);
        return;
    case EXPR_CALL: {
        /*
         * gen_expr(out, arg) + push rax, for each argument
         *     pop <argreg>, in reverse, for each argument
         *     mov r11, rsp
         *     and r11, 15
         *     mov [rbp-stack], r11
         *     sub rsp, r11
         *     call function_name
         *     mov r11, [rbp-stack]
         *     add rsp, r11
         */
        static const char *argregs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
        int argc = expr->call.argc;
        if (argc > 6)
            error("calls to '%s' with more than 6 arguments are unsupported", expr->call.name);

        for (int i = 0; i < argc; i++) {
            gen_expr(out, expr->call.args[i]);
            fprintf(out, "    push rax\n");
        }

        for (int i = argc - 1; i >= 0; i--)
            fprintf(out, "    pop %s\n", argregs[i]);

        fprintf(out, "    mov r11, rsp\n");
        fprintf(out, "    and r11, 15\n");
        fprintf(out, "    mov [rbp-%d], r11\n", stack);
        fprintf(out, "    sub rsp, r11\n");
        fprintf(out, "    call %s\n", symbol_get(symbols, expr->call.symbol)->function->name);
        fprintf(out, "    mov r11, [rbp-%d]\n", stack);
        fprintf(out, "    add rsp, r11\n");
        return;
    }
    default:
        error("internal error: unknown expression type %d", expr->type);
    }
}

static void gen_ret_stmt(FILE *out, Stmt *stmt) {
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
     *     mov dword [rbp+offset], eax
     */
    if (stmt->decl_stmt.initializer) {
        gen_expr(out, stmt->decl_stmt.initializer);
        fprintf(out, "    mov dword [rbp%+d], eax\n", symbol_get(symbols, stmt->decl_stmt.symbol)->offset);
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

static void gen_while_stmt(FILE *out, Stmt *stmt) {
    /*
     * .Lcontinuelabel:
     * gen_expr(out, stmt->while_stmt.cond)
     *     test eax, eax
     *     je .Lbreaklabel
     * gen_stmt(out, stmt->while_stmt.body)
     *     jmp .Lcontinuelabel
     * .Lbreaklabel:
     */
    int label = gen_label();
    LoopCtx loop = {
        .label  = label,
        .parent = ctx
    };

    ctx = &loop;
    fprintf(out, ".Lcontinue%d:\n", label);
    gen_expr(out, stmt->while_stmt.cond);
    fprintf(out, "    test eax, eax\n");
    fprintf(out, "    je .Lbreak%d\n", label);
    gen_stmt(out, stmt->while_stmt.body);
    fprintf(out, "    jmp .Lcontinue%d\n", label);
    fprintf(out, ".Lbreak%d:\n", label);
    ctx = loop.parent;
}

static void gen_for_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_stmt(out, stmt->for_stmt.init)
     * .Lcondlabel:
     * gen_expr(out, stmt->for_stmt.cond)
     *     test eax, eax
     *     je .Lbreaklabel
     * gen_stmt(out, stmt->for_stmt.body)
     * .Lcontinuelabel:
     * gen_expr(out, stmt->for_stmt.inc)
     *     jmp .Lcondlabel
     * .Lbreaklabel:
     */
    int label = gen_label();
    LoopCtx loop = {
        .label  = label,
        .parent = ctx
    };

    ctx = &loop;
    gen_stmt(out, stmt->for_stmt.init);
    fprintf(out, ".Lcond%d:\n", label);
    gen_expr(out, stmt->for_stmt.cond);
    fprintf(out, "    test eax, eax\n");
    fprintf(out, "    je .Lbreak%d\n", label);
    gen_stmt(out, stmt->for_stmt.body);
    fprintf(out, ".Lcontinue%d:\n", label);
    gen_expr(out, stmt->for_stmt.inc);
    fprintf(out, "    jmp .Lcond%d\n", label);
    fprintf(out, ".Lbreak%d:\n", label);
    ctx = loop.parent;
}

static void gen_break_stmt(FILE *out) {
    /*
     *     jmp .Lbreaklabel
     */
    if (!ctx)
        error("'break' outside of a loop");

    fprintf(out, "    jmp .Lbreak%d\n", ctx->label);
}

static void gen_continue_stmt(FILE *out) {
    /*
     *     jmp .Lcontinuelabel
     */
    if (!ctx)
        error("'continue' outside of a loop");

    fprintf(out, "    jmp .Lcontinue%d\n", ctx->label);
}

static void gen_expr_stmt(FILE *out, Stmt *stmt) {
    /*
     * gen_expr(out, stmt->expr_stmt)
     */
    gen_expr(out, stmt->expr_stmt);
}

static void gen_stmt(FILE *out, Stmt *stmt) {
    switch (stmt->type) {
    case STMT_RETURN:   gen_ret_stmt(out, stmt);                                        return;
    case STMT_DECL:     gen_decl_stmt(out, stmt);                                       return;
    case STMT_IF:       gen_if_stmt(out, stmt);                                         return;
    case STMT_BLOCK:    gen_block(out, stmt->block_stmt);                               return;
    case STMT_WHILE:    gen_while_stmt(out, stmt);                                      return;
    case STMT_FOR:      gen_for_stmt(out, stmt);                                        return;
    case STMT_BREAK:    gen_break_stmt(out);                                            return;
    case STMT_CONTINUE: gen_continue_stmt(out);                                         return;
    case STMT_EXPR:     gen_expr_stmt(out, stmt);                                       return;
    default:            error("internal error: unknown statement type %d", stmt->type); return;
    }
}

static void gen_block(FILE *out, Block *block) {
    for (int i = 0; i < block->stmtc; i++)
        gen_stmt(out, block->stmts[i]);
}

static void gen_function(FILE *out, Function *function) {
    /*
     * global function->name
     * function->name:
     *     push rbp
     *     mov rbp, rsp
     *     mov dword [rbp+offset], <argreg>, for each parameter
     * gen_block(out, function->body)
     * .return:
     *     mov rsp, rbp
     *     pop rbp
     *     ret
     */
    static const char *argregs[] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
    if (function->paramc > 6)
        error("'%s' has more than 6 parameters", function->name);

    stack = function->stack + 8;

    fprintf(out, "global %s\n", function->name);
    fprintf(out, "%s:\n", function->name);
    fprintf(out, "    push rbp\n");
    fprintf(out, "    mov rbp, rsp\n");
    fprintf(out, "    sub rsp, %d\n", stack);
    for (int i = 0; i < function->paramc; i++)
        fprintf(out, "    mov dword [rbp%+d], %s\n", -4 * (i + 1), argregs[i]);
    gen_block(out, function->body);
    fprintf(out, ".return:\n");
    fprintf(out, "    mov rsp, rbp\n");
    fprintf(out, "    pop rbp\n");
    fprintf(out, "    ret\n");
}

void gen(FILE *out, Program *program, SymbolTable *table) {
    /*
     * section .text
     * gen_function(out, function), for each defined function
     */
    symbols = table;
    fprintf(out, "section .text\n");
    for (int i = 0; i < program->functionc; i++) {
        if (program->functions[i]->body)
            gen_function(out, program->functions[i]);
    }
}

void gen_free() {
    for (LoopCtx *cur = ctx; cur;) {
        LoopCtx *parent = cur->parent;
        free(cur);
        cur = parent;
    }
}