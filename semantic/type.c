#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type.h>

#define DEF_PTR_SZ    8
#define DEFAULT_BUCKET_SZ 64

static void fatal(char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static void *grow(void *p, size_t *cap, size_t elem) {
    *cap = *cap ? *cap << 1 : 1;
    void *tmp = realloc(p, *cap * elem);
    if (!tmp)
        fatal("out of memory");

    return tmp;
}

static uint64_t mix(uint64_t h, uint64_t v) {
    h ^= v;
    h *= 0x100000001b3ULL;
    return h;
}

static uint64_t hash(Type *type, TypeId *params) {
    uint64_t h = mix(0xcbf29ce484222325ULL, (uint64_t)type->kind);
    switch (type->kind) {
    case TYPE_PTR:
        return mix(h, type->pointer.to);
    case TYPE_ARRAY:
        h = mix(h, type->array.elem);
        return mix(h, type->array.len);
    case TYPE_FUNCTION:
        h = mix(h, type->function.ret);
        h = mix(h, (uint64_t)type->function.paramc);
        for (int i = 0; i < type->function.paramc; i++)
            h = mix(h, params[i]);
        return h;
    default:
        return h;
    }
}

static bool equal(TypeTable *table, Type *a, Type *b, TypeId *b_params) {
    if (a->kind != b->kind)
        return false;

    switch (a->kind) {
    case TYPE_PTR:
        return a->pointer.to == b->pointer.to;
    case TYPE_ARRAY:
        return a->array.elem == b->array.elem && a->array.len == b->array.len;
    case TYPE_FUNCTION:
        return a->function.ret    == b->function.ret    &&
               a->function.paramc == b->function.paramc &&
               !memcmp(table->params + a->function.params, b_params, (size_t)a->function.paramc * sizeof(TypeId));
    default:
        return true;
    }
}

static void insert(TypeTable *table, TypeId id, uint64_t h) {
    size_t mask = table->bucketc - 1;
    size_t i = (size_t)h & mask;
    while (table->buckets[i] != TYPE_INVALID)
        i = (i + 1) & mask;

    table->buckets[i] = id;
}

static void rehash(TypeTable *table) {
    free(table->buckets);
    table->bucketc <<= 1;
    table->buckets = malloc(table->bucketc * sizeof(TypeId));
    if (!table->buckets)
        fatal("out of memory");

    memset(table->buckets, 0xff, table->bucketc * sizeof(TypeId)); /* all TYPE_INVALID */
    for (size_t id = 0; id < table->typec; id++) {
        Type *type = &table->types[id];
        TypeId *params = type->kind == TYPE_FUNCTION ? table->params + type->function.params : NULL;
        insert(table, (TypeId)id, hash(type, params));
    }
}

static TypeId append(TypeTable *table, Type type, TypeId *params) {
    if (table->typec == table->type_cap)
        table->types = grow(table->types, &table->type_cap, sizeof(Type));

    if (table->typec >= UINT32_MAX)
        fatal("out of types");

    if (type.kind == TYPE_FUNCTION) {
        while (table->paramc + (size_t)type.function.paramc > table->param_cap)
            table->params = grow(table->params, &table->param_cap, sizeof(TypeId));

        type.function.params = (uint32_t)table->paramc;
        memcpy(table->params + table->paramc, params, (size_t)type.function.paramc * sizeof(TypeId));
        table->paramc += (size_t)type.function.paramc;
    }

    TypeId id = (TypeId)table->typec++;
    table->types[id] = type;
    return id;
}

static TypeId intern(TypeTable *table, Type type, TypeId *params) {
    uint64_t h = hash(&type, params);
    size_t mask = table->bucketc - 1;
    for (size_t i = (size_t)h & mask;; i = (i + 1) & mask) {
        TypeId id = table->buckets[i];
        if (id == TYPE_INVALID) {
            id = append(table, type, params);
            table->buckets[i] = id;
            if (table->typec * 4 > table->bucketc * 3)
                rehash(table);

            return id;
        }

        if (equal(table, &table->types[id], &type, params))
            return id;
    }
}

TypeId type_ptr(TypeTable *table, TypeId to) {
    Type type = {
        .kind       = TYPE_PTR,
        .sz         = DEF_PTR_SZ,
        .align      = DEF_PTR_SZ,
        .pointer.to = to
    };
    return intern(table, type, NULL);
}

TypeId type_array(TypeTable *table, TypeId elem, size_t len) {
    Type *e = type_get(table, elem);
    Type type = {
        .kind       = TYPE_ARRAY,
        .sz         = e->sz * len,
        .align      = e->align,
        .array.elem = elem,
        .array.len  = len
    };
    return intern(table, type, NULL);
}

TypeId type_function(TypeTable *table, TypeId ret, TypeId *params, int paramc) {
    Type type = {
        .kind            = TYPE_FUNCTION,
        .sz              = 0,
        .align           = 1,
        .function.ret    = ret,
        .function.params = 0,
        .function.paramc = paramc
    };
    return intern(table, type, params);
}

TypeId type_param(TypeTable *table, TypeId id, int i) {
    Type *type = type_get(table, id);
    assert(type->kind == TYPE_FUNCTION);
    assert(i >= 0 && i < type->function.paramc);
    return table->params[type->function.params + (uint32_t)i];
}

Type *type_get(TypeTable *table, TypeId id) {
    assert(id < table->typec);
    return &table->types[id];
}

void type_table_free(TypeTable *table) {
    free(table->types);
    free(table->buckets);
    free(table->params);
}

void type_table_init(TypeTable *table) {
    table->types     = NULL;
    table->typec     = 0;
    table->type_cap  = 0;
    table->bucketc   = DEFAULT_BUCKET_SZ;
    table->buckets   = malloc(table->bucketc * sizeof(TypeId));
    if (!table->buckets)
        fatal("out of memory");

    /* all TYPE_INVALID */
    memset(table->buckets, 0xff, table->bucketc * sizeof(TypeId));
    table->params    = NULL;
    table->paramc    = 0;
    table->param_cap = 0;

    static struct { TypeKind kind; size_t sz, align; } primitives[] = {
        { TYPE_VOID, 0, 1 },
        { TYPE_BOOL, 1, 1 },
        { TYPE_CHAR, 1, 1 },
        { TYPE_INT,  4, 4 },
        { TYPE_LONG, 8, 8 }
    };

    for (int i = 0; i < TYPE_PRIMITIVE_CNT; i++) {
        Type type = {
            .kind  = primitives[i].kind,
            .sz    = primitives[i].sz,
            .align = primitives[i].align
        };
        TypeId id = intern(table, type, NULL);
        assert(id == type_primitive(primitives[i].kind));
        (void)id;
    }
}