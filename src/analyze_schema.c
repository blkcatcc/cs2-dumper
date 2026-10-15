#include "dumper.h"

/* Struct offsets are taken from the live game; they move on a CS2 update. */

#define SCHEMASYSTEM_TYPE_SCOPES 0x1F0
#define SCHEMASYSTEM_REG_COUNT 0x280

#define TYPESCOPE_NAME 0x008
#define TYPESCOPE_CLASS_BINDINGS 0x560
#define TYPESCOPE_ENUM_BINDINGS 0x4DD0

#define UTLVECTOR_COUNT 0x00
#define UTLVECTOR_DATA 0x08

#define CLASS_NAME 0x08
#define CLASS_MODULE_NAME 0x18
#define CLASS_FIELD_COUNT 0x24
#define CLASS_STATIC_MD_COUNT 0x26
#define CLASS_FIELDS 0x30
#define CLASS_BASE_CLASSES 0x40
#define CLASS_STATIC_MD 0x48
#define CLASS_FIELD_SIZE 0x20

#define BASECLASS_CLASS 0x18
#define BASECLASS_NAME 0x10

#define FIELD_NAME 0x00
#define FIELD_TYPE 0x08
#define FIELD_OFFSET 0x10

#define TYPE_NAME 0x08

#define ENUM_NAME 0x08
#define ENUM_ALIGNMENT 0x19
#define ENUM_COUNT 0x1C
#define ENUM_ENUMERATORS 0x20
#define ENUMERATOR_NAME 0x00
#define ENUMERATOR_VALUE 0x08
#define ENUMERATOR_SIZE 0x20

#define MD_NAME 0x00
#define MD_NETWORK_VALUE 0x08
#define NETVALUE_NAME_PTR 0x00
#define NETVALUE_VAR_NAME 0x00
#define NETVALUE_VAR_TYPE 0x08

/* UtlTsHash layout, used for the class and enum binding sets. */
#define TSHASH_ENTRY_MEM 0x0000
#define TSHASH_BUCKETS 0x0060      /* offset of the bucket array */
#define TSHASH_BUCKET_SIZE 0x18
#define TSHASH_BUCKET_COUNT 768    /* UtlTsHash<D, 768> */
#define MEMPOOL_BLOCKS_ALLOCATED 0x0C
#define MEMPOOL_PEAK_ALLOCATED 0x10
#define MEMPOOL_FREE_BLOCKS 0x20
#define MEMPOOL_BLOB_HEAD 0x48
#define TSHASH_ALLOC_BLOB_DATA 0x10

#define UTLLIST_NEXT 0x00

typedef struct {
    const Mem *mem;
    u64 *seen;
    size_t nseen, cap;
} PtrSet;

static int seen_add(PtrSet *s, u64 ptr) {
    for (size_t i = 0; i < s->nseen; i++)
        if (s->seen[i] == ptr) return 0;
    if (s->nseen == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 256;
        s->seen = xrealloc(s->seen, s->cap * sizeof(*s->seen));
    }
    s->seen[s->nseen++] = ptr;
    return 1;
}

/* Walks an UtlTsHash: the committed bucket chains, then the free blob list.
   The two lists are collected independently and deduplicated afterwards, which
   is what the reference does; a shared running cap would stop scanning buckets
   early and lose entries. */
static void push_unique(u64 **out, size_t *n, size_t *cap, PtrSet *seen, u64 data) {
    if (!data || !seen_add(seen, data)) return;
    if (*n == *cap) {
        *cap *= 2;
        *out = xrealloc(*out, *cap * sizeof(**out));
    }
    (*out)[(*n)++] = data;
}

static u64 *tshash_elements(const Mem *mem, u64 hash, size_t *out_n) {
    size_t cap = 128, n = 0;
    u64 *out = xcalloc(cap, sizeof(*out));
    PtrSet seen = { mem, NULL, 0, 0 };

    u32 used = mem_read_u32(mem, hash + TSHASH_ENTRY_MEM + MEMPOOL_BLOCKS_ALLOCATED);
    for (size_t b = 0; b < TSHASH_BUCKET_COUNT; b++) {
        u64 bucket = hash + TSHASH_BUCKETS + b * TSHASH_BUCKET_SIZE;
        /* first_uncommitted at 0x10; `first` at 0x08 is only the committed head
           and misses entries the schema system has not re-hashed yet. */
        u64 node = mem_read_u64(mem, bucket + 0x10);
        size_t chain = 0;
        while (node && chain++ < 100000) {
            push_unique(&out, &n, &cap, &seen, mem_read_u64(mem, node + 0x10));
            if (n >= used) break; /* per-chain cap, as in the reference */
            node = mem_read_u64(mem, node + 0x08);
        }
    }

    u32 peak = mem_read_u32(mem, hash + TSHASH_ENTRY_MEM + MEMPOOL_PEAK_ALLOCATED);
    u64 blob = mem_read_u64(mem, hash + TSHASH_ENTRY_MEM + MEMPOOL_FREE_BLOCKS + UTLLIST_NEXT);
    size_t guard = 0;
    while (blob && guard++ < 100000) {
        push_unique(&out, &n, &cap, &seen, mem_read_u64(mem, blob + TSHASH_ALLOC_BLOB_DATA));
        if (n >= peak) break;
        blob = mem_read_u64(mem, blob + UTLLIST_NEXT);
    }

    free(seen.seen);
    *out_n = n;
    return out;
}


/* Strips spaces, matching the reference dump's type strings. */
static char *strip_spaces(const char *s) {
    size_t n = strlen(s);
    char *o = xmalloc(n + 1);
    size_t k = 0;
    for (size_t i = 0; i < n; i++)
        if (s[i] != ' ') o[k++] = s[i];
    o[k] = 0;
    return o;
}

static void read_class(const Mem *mem, u64 binding, Class *out) {
    memset(out, 0, sizeof(*out));

    char *name = mem_read_str(mem, mem_read_u64(mem, binding + CLASS_NAME), 128);
    if (!name[0]) {
        free(name);
        return;
    }
    out->name = name;

    u64 base_classes = mem_read_u64(mem, binding + CLASS_BASE_CLASSES);
    if (base_classes) {
        u64 base = mem_read_u64(mem, base_classes + BASECLASS_CLASS);
        if (base) {
            char *pname = mem_read_str(mem, mem_read_u64(mem, base + BASECLASS_NAME), 128);
            if (pname[0]) out->parent = pname;
            else free(pname);
        }
    }

    u16 field_count = (u16)mem_read_u32(mem, binding + CLASS_FIELD_COUNT);
    if (field_count > 0x1000) field_count = 0;
    u64 fields = mem_read_u64(mem, binding + CLASS_FIELDS);
    if (fields && field_count) {
        out->fields = xcalloc(field_count, sizeof(*out->fields));
        for (u16 i = 0; i < field_count; i++) {
            u64 f = fields + (u64)i * CLASS_FIELD_SIZE;
            u64 type = mem_read_u64(mem, f + FIELD_TYPE);
            if (!type) break;
            char *fname = mem_read_str(mem, mem_read_u64(mem, f + FIELD_NAME), 128);
            char *tname = mem_read_str(mem, mem_read_u64(mem, type + TYPE_NAME), 128);
            char *clean = strip_spaces(tname);
            free(tname);

            Field *fld = &out->fields[out->nfields++];
            fld->name = fname;
            fld->type_name = clean;
            fld->offset = (i32)mem_read_u32(mem, f + FIELD_OFFSET);
        }
    }

    u16 md_count = (u16)mem_read_u32(mem, binding + CLASS_STATIC_MD_COUNT);
    if (md_count > 0x1000) md_count = 0;
    u64 md = mem_read_u64(mem, binding + CLASS_STATIC_MD);
    if (md && md_count) {
        out->md = xcalloc(md_count, sizeof(*out->md));
        for (u16 i = 0; i < md_count; i++) {
            u64 e = md + (u64)i * 16;
            u64 net = mem_read_u64(mem, e + MD_NETWORK_VALUE);
            if (!net) break;

            char *mname = mem_read_str(mem, mem_read_u64(mem, e + MD_NAME), 128);
            Metadata *m = &out->md[out->nmd];

            if (strcmp(mname, "MNetworkChangeCallback") == 0) {
                m->kind = MD_CHANGE_CB;
                m->name = mem_read_str(mem, mem_read_u64(mem, net + NETVALUE_NAME_PTR), 128);
            } else if (strcmp(mname, "MNetworkVarNames") == 0) {
                m->kind = MD_VAR_NAMES;
                m->name = mem_read_str(mem, mem_read_u64(mem, net + NETVALUE_VAR_NAME), 128);
                char *t = mem_read_str(mem, mem_read_u64(mem, net + NETVALUE_VAR_TYPE), 128);
                m->type_name = strip_spaces(t);
                free(t);
            } else {
                m->kind = MD_UNKNOWN;
                m->name = mname;
                mname = NULL;
            }
            free(mname);
            out->nmd++;
        }
    }
}

static int read_enum(const Mem *mem, u64 binding, Enum *out) {
    memset(out, 0, sizeof(*out));

    char *name = mem_read_str(mem, mem_read_u64(mem, binding + ENUM_NAME), 128);
    if (!name[0]) {
        free(name);
        return 0;
    }
    out->name = name;

    out->alignment = (u8)mem_read_u32(mem, binding + ENUM_ALIGNMENT);
    out->size = (u16)mem_read_u32(mem, binding + ENUM_COUNT);

    u64 enumerators = mem_read_u64(mem, binding + ENUM_ENUMERATORS);
    if (enumerators && out->size) {
        out->members = xcalloc(out->size, sizeof(*out->members));
        for (u16 i = 0; i < out->size; i++) {
            u64 e = enumerators + (u64)i * ENUMERATOR_SIZE;
            EnumMember *m = &out->members[out->nmembers++];
            m->name = mem_read_str(mem, mem_read_u64(mem, e + ENUMERATOR_NAME), 128);
            m->value = (i64)mem_read_u64(mem, e + ENUMERATOR_VALUE);
        }
    }
    return 1;
}

static int cmp_class(const void *a, const void *b) {
    return strcmp(((const Class *)a)->name, ((const Class *)b)->name);
}

static int cmp_enum(const void *a, const void *b) {
    return strcmp(((const Enum *)a)->name, ((const Enum *)b)->name);
}


/* Finds the live SchemaSystem by walking the module's interface list and
   reading the prologue of the SchemaSystem_001 create function. */
static u64 find_schema_system(const Mem *mem, const Module *mod) {
    size_t len = 0;
    u8 *buf = read_module(mem, mod, &len);
    if (!buf) return 0;

    Image view;
    memset(&view, 0, sizeof(view));
    view.data = buf;
    view.len = len;
    view.base = mod->base;

    u64 export = image_export(&view, "CreateInterface");
    if (!export) {
        free(buf);
        return 0;
    }

    size_t start = (size_t)export;
    size_t end = start + 0x40;
    if (end > len) end = len;

    Pattern pat;
    u64 list_head = 0;
    int have = 0;
    if (start < end && pattern_parse(&pat, "48 8B 1D ? ? ? ? 48 85 DB")) {
        size_t off = 0;
        have = pattern_scan(&pat, buf + start, end - start, &off);
        pattern_free(&pat);
        if (have) {
            u64 list_ptr = resolve_rip(mem, mod->base + start + off, 3, 7);
            list_head = mem_read_u64(mem, list_ptr);
        }
    }
    free(buf);
    if (!have || !list_head) return 0;

    u64 cur = list_head;
    size_t guard = 0;
    while (cur && guard++ < 100000) {
        char *name = mem_read_str(mem, mem_read_u64(mem, cur + 0x08), 128);
        int is_target = strcmp(name, "SchemaSystem_001") == 0;
        free(name);

        if (is_target) {
            u64 create_fn = mem_read_u64(mem, cur);
            u8 code[7] = { 0 };
            if (mem_read(mem, create_fn, code, 7) && code[0] == 0x48 &&
                (code[1] == 0x8B || code[1] == 0x8D) && code[2] == 0x05) {
                i32 disp;
                memcpy(&disp, code + 3, 4);
                u64 instance = create_fn + 7 + (i64)disp;

                if (code[1] == 0x8D) {
                    u64 reg = mem_read_u32(mem, instance + SCHEMASYSTEM_REG_COUNT);
                    if (reg > 0) return instance;
                } else {
                    u64 ptr = mem_read_u64(mem, instance);
                    if (ptr) {
                        u64 reg = mem_read_u32(mem, ptr + SCHEMASYSTEM_REG_COUNT);
                        if (reg > 0) return ptr;
                    }
                }
            }
        }
        cur = mem_read_u64(mem, cur + 0x10);
    }
    return 0;
}

SchemaMap analyze_schemas(const Mem *mem, const Modules *mods) {
    SchemaMap map;
    size_t cap = 8;
    map.v = xcalloc(cap, sizeof(*map.v));
    map.n = 0;

    const Module *mod = modules_get(mods, "libschemasystem.so");
    if (!mod) {
        fprintf(stderr, "warning: skipping schemas: libschemasystem.so not found\n");
        return map;
    }

    u64 sys = find_schema_system(mem, mod);
    if (!sys) {
        fprintf(stderr, "error: unable to read the schema system address\n");
        return map;
    }

    u64 scopes = sys + SCHEMASYSTEM_TYPE_SCOPES;
    i32 count = (i32)mem_read_u32(mem, scopes + UTLVECTOR_COUNT);
    u64 data = mem_read_u64(mem, scopes + UTLVECTOR_DATA);
    if (count <= 0 || count > 0x1000 || !data) return map;

    for (i32 i = 0; i < count; i++) {
        u64 scope_ptr = mem_read_u64(mem, data + (u64)i * 8);
        if (!scope_ptr) continue;

        char *module_name = mem_read_str(mem, scope_ptr + TYPESCOPE_NAME, 256);
        if (!module_name[0]) {
            free(module_name);
            continue;
        }

        if (map.n == cap) {
            cap *= 2;
            map.v = xrealloc(map.v, cap * sizeof(*map.v));
            memset(map.v + map.n, 0, (cap - map.n) * sizeof(*map.v));
        }
        Schema *sc = &map.v[map.n];
        sc->module = module_name;

        size_t n = 0;
        u64 *classes = tshash_elements(mem, scope_ptr + TYPESCOPE_CLASS_BINDINGS, &n);
        sc->classes = xcalloc(n ? n : 1, sizeof(*sc->classes));
        for (size_t k = 0; k < n; k++) {
            Class c;
            read_class(mem, classes[k], &c);
            if (!c.name) continue;
            sc->classes[sc->nclasses++] = c;
        }
        free(classes);
        qsort(sc->classes, sc->nclasses, sizeof(*sc->classes), cmp_class);

        u64 *enums = tshash_elements(mem, scope_ptr + TYPESCOPE_ENUM_BINDINGS, &n);
        sc->enums = xcalloc(n ? n : 1, sizeof(*sc->enums));
        for (size_t k = 0; k < n; k++) {
            Enum e;
            if (read_enum(mem, enums[k], &e)) sc->enums[sc->nenums++] = e;
        }
        free(enums);
        qsort(sc->enums, sc->nenums, sizeof(*sc->enums), cmp_enum);

        if (sc->nclasses == 0 && sc->nenums == 0) {
            free(sc->classes);
            free(sc->enums);
            free(sc->module);
            continue;
        }
        fprintf(stderr, "info: scope %s: %zu classes, %zu enums\n", sc->module, sc->nclasses,
                sc->nenums);
        map.n++;
    }

    return map;
}
