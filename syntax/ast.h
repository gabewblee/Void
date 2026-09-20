#pragma once

#include <lexer.h>
#include <symtbl.h>

typedef enum {
    EXPR_INT,    /* Integer constant    */
    EXPR_UNARY,  /* Unary operation     */
    EXPR_BINARY, /* Binary operaton     */
    EXPR_ID,     /* Identifier          */
    EXPR_ASSIGN  /* Variable assignment */
} ExprType;

typedef struct Expr Expr;

struct Expr {
    ExprType type; /* Expression type */
    union {
        long integer; /* Expression integer */
        struct {
            TokenType op;      /* Unary operation */
            Expr*     operand; /* Unary operand   */
        } unary;
        struct {
            TokenType op;    /* Binary operation */
            Expr*     left;  /* Left operand     */
            Expr*     right; /* Right operand    */
        } binary;
        struct {
            char   *name;   /* Identifier name   */
            Symbol *symbol; /* Identifier symbol */
        } id;
        struct {
            Expr *target; /* Assignment target */
            Expr *val;    /* Assignment value  */
        } assign;
    };
};

typedef enum {
    STMT_RETURN,   /* Return statement      */
    STMT_DECL,     /* Declaration statement */
    STMT_IF,       /* If statement          */
    STMT_BLOCK,    /* Block statement       */
    STMT_WHILE,    /* While statement       */
    STMT_BREAK,    /* Break statement       */
    STMT_CONTINUE, /* Continue statement    */
    STMT_EXPR      /* Expression statement  */
} StmtType;

typedef struct Stmt Stmt;
typedef struct Block Block;

struct Stmt {
    StmtType type; /* Statement type */
    union {
        Expr *ret_stmt; /* Return value */
        struct {
            char   *name;        /* Variable name          */
            Expr   *initializer; /* Variable initial value */
            Symbol *symbol;      /* Variable symbol        */
        } decl_stmt;
        struct {
            Expr *cond;        /* If condition */
            Stmt *then_branch; /* Then branch  */
            Stmt *else_branch; /* Else branch  */
        } if_stmt;
        Block *block_stmt;
        struct {
            Expr *cond; /* While loop condition */
            Stmt *body; /* While loop body      */
        } while_stmt;
        Expr *expr_stmt; /* Expression value */
    };
};

struct Block {
    Stmt **stmts; /* Statement list  */
    int    cnt;   /* Statement count */
};

typedef struct Function Function;

struct Function {
    char  *name;  /* Function name       */
    Block *body;  /* Function body       */
    int    stack; /* Function stack size */
};

typedef struct Program Program;

struct Program {
    Function *function; /* Program function */
};

/**
 * ast_build_integer_expr_node - Builds an integer expression node.
 * @integer: The node's integer value.
 * Returns: The integer expression node.
 */
Expr *ast_build_integer_expr_node(long integer);

/**
 * ast_build_unary_expr_node - Builds a unary expression node.
 * @op: The node's unary operation.
 * @operand: The node's unary operand.
 * Returns: The unary expression node.
 */
Expr *ast_build_unary_expr_node(TokenType op, Expr *operand);

/**
 * ast_build_binary_expr_node - Builds a binary expression node.
 * @op: The node's binary operation.
 * @left: The node's left operand.
 * @right: The node's right operand.
 * Returns: The binary expression node.
 */
Expr *ast_build_binary_expr_node(TokenType op, Expr *left, Expr *right);

/**
 * ast_build_id_expr_node - Builds an identifier expression node.
 * @name: The identifier's name.
 * Returns: The identifier expression node.
 */
Expr *ast_build_id_expr_node(char *name);

/**
 * ast_build_assign_expr_node - Builds an assignment expression node.
 * @target: The node's target.
 * @val: The node's value.
 * Returns: The assignment expression node.
 */
Expr *ast_build_assign_expr_node(Expr *target, Expr *val);

/**
 * ast_build_ret_stmt_node - Builds a return statement node.
 * @return_expr: The node's return value.
 * Returns: The return statement node.
 */
Stmt *ast_build_ret_stmt_node(Expr *return_expr);

/**
 * ast_build_decl_stmt_node - Builds a declaration statement node.
 * @name: The variable's name.
 * @initializer: The variable's initial value.
 * Returns: The declaration statement node.
 */
Stmt *ast_build_decl_stmt_node(char *name, Expr *initializer);

/**
 * ast_build_if_stmt_node - Builds an if statement node.
 * @cond: The if condition.
 * @then_branch: The then branch.
 * @else_branch: The else branch.
 * Returns: The if statement node.
 */
Stmt *ast_build_if_stmt_node(Expr *cond, Stmt *then_branch, Stmt *else_branch);

/**
 * ast_build_block_stmt_node - Builds a block statement node.
 * @block: The block node.
 * Returns: The block statement node.
 */
Stmt *ast_build_block_stmt_node(Block *block);

/**
 * ast_build_while_stmt_node - Builds a while statement node.
 * @cond: The while loop condition.
 * @body: The while loop body.
 * Returns: The while statement node.
 */
Stmt *ast_build_while_stmt_node(Expr *cond, Stmt *body);

/**
 * ast_build_break_stmt_node - Builds a break statement node.
 * Returns: The break statement node.
 */
Stmt *ast_build_break_stmt_node(void);

/**
 * ast_build_continue_stmt_node - Builds a continue statement node.
 * Returns: The continue statement node.
 */
Stmt *ast_build_continue_stmt_node(void);

/**
 * ast_build_expr_stmt_node - Builds a expression statement node.
 * @expr: The statement's expression.
 * Returns: The expression statement node.
 */
Stmt *ast_build_expr_stmt_node(Expr *expr);

/**
 * ast_build_block_node - Builds a block node.
 * @stmts: The statement list.
 * @cnt: The statement count.
 */
Block *ast_build_block_node(Stmt **stmts, int cnt);

/**
 * ast_build_function_node - Builds a function node.
 * @name: The function's name.
 * @body: The function's body.
 * Returns: The function node.
 */
Function *ast_build_function_node(char *name, Block *body);

/**
 * ast_build_program_node - Builds a program node.
 * @function: The program's entry function.
 * Returns: The program node.
 */
Program *ast_build_program_node(Function *function);

/**
 * ast_free_program_node - Frees the program's allocated memory.
 * @program: The program to free.
 */
void ast_free_program_node(Program *program);