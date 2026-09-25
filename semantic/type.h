#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint32_t TypeId;

#define TYPE_INVALID UINT32_MAX

typedef enum {
    /* Primitive types */
    TYPE_VOID,    /* void type           */
    TYPE_BOOL,    /* 1-byte boolean type */
    TYPE_CHAR,    /* 1-byte integer type */
    TYPE_INT,     /* 4-byte integer type */
    TYPE_LONG,    /* 8-byte integer type */

    /* Compound types  */
    TYPE_PTR,     /* Pointer type        */
    TYPE_ARRAY,   /* Array type          */
    TYPE_FUNCTION /* Function type       */
} TypeKind;

#define TYPE_PRIMITIVE_CNT ((int)TYPE_LONG + 1)

typedef struct {
    TypeKind kind;  /* Type kind               */
    size_t   sz;    /* Type size in bytes      */
    size_t   align; /* Type alignment in bytes */
    union {
        struct {
            TypeId to; /* Pointee type */
        } pointer;
        struct {
            TypeId elem; /* Element type  */
            size_t len;  /* Element count */
        } array;
        struct {
            TypeId   ret;    /* Return type     */
            uint32_t params; /* Parameter start */
            int      paramc; /* Parameter count */
        } function;
    };
} Type;

typedef struct {
    Type   *types;     /* Type storage            */
    size_t  typec;     /* Type count              */
    size_t  type_cap;  /* Type capacity           */

    TypeId *buckets;   /* Bucket storage          */
    size_t  bucketc;   /* Bucket count            */

    TypeId *params;    /* Parameter pool          */
    size_t  paramc;    /* Parameter pool length   */
    size_t  param_cap; /* Parameter pool capacity */
} TypeTable;

/**
 * type_ptr - Interns a pointer type.
 * @table: The type table.
 * @to: The pointee type.
 * Returns: The id of @to*.
 */
TypeId type_ptr(TypeTable *table, TypeId to);

/**
 * type_array - Interns an array type.
 * @table: The type table.
 * @elem: The element type.
 * @len: The element count.
 * Returns: The id of @elem[@len].
 */
TypeId type_array(TypeTable *table, TypeId elem, size_t len);

/**
 * type_function - Interns a function type.
 * @table: The type table.
 * @ret: The return type.
 * @params: The parameter types.
 * @paramc: The parameter count.
 * Returns: The id of @ret(@params...).
 */
TypeId type_function(TypeTable *table, TypeId ret, TypeId *params, int paramc);

/**
 * type_primitive - Gets the id of @kind.
 * @kind: The primitive type kind.
 * Returns: The id of the primitive type.
 */
static inline TypeId type_primitive(TypeKind kind) {
    assert((int)kind < TYPE_PRIMITIVE_CNT);
    return (TypeId)kind;
}

/**
 * type_param - Gets the @i-th parameter type of function type @id.
 * @table: The type table.
 * @id: The function type.
 * @i: The parameter index.
 * Returns: The id of the @i-th parameter.
 */
TypeId type_param(TypeTable *table, TypeId id, int i);

/**
 * type_get - Gets the type @id from @table.
 * @table: The type table.
 * @id: The type id.
 * Returns: The type @id from @table.
 */
Type *type_get(TypeTable *table, TypeId id);

/**
 * type_table_free - Frees @table's allocated memory.
 * @table: The type table to free.
 */
void type_table_free(TypeTable *table);

/**
 * type_table_init - Initializes @table with primitive types.
 * @table: The type table to initialize.
 */
void type_table_init(TypeTable *table);
