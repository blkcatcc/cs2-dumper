#include "dumper.h"

/* Schema classes and enums per module, in all five formats. */

static const char *align_type(const Enum *e, const char *lang) {
    if (strcmp(lang, "hpp") == 0) {
        switch (e->alignment) {
        case 1: return "uint8_t";
        case 2: return "uint16_t";
        case 4: return "uint32_t";
        case 8: return "uint64_t";
        }
    } else if (strcmp(lang, "cs") == 0) {
        switch (e->alignment) {
        case 1: return "byte";
        case 2: return "ushort";
        case 4: return "uint";
        case 8: return "ulong";
        }
    } else if (strcmp(lang, "rs") == 0 || strcmp(lang, "zig") == 0) {
        switch (e->alignment) {
        case 1: return "u8";
        case 2: return "u16";
        case 4: return "u32";
        case 8: return "u64";
        }
    }
    return NULL;
}

static const char *json_align_type(u8 alignment) {
    switch (alignment) {
    case 1: return "uint8";
    case 2: return "uint16";
    case 4: return "uint32";
    case 8: return "uint64";
    }
    return "unknown";
}

static u64 align_max(const char *t) {
    if (strcmp(t, "u8") == 0 || strcmp(t, "uint8_t") == 0 || strcmp(t, "byte") == 0) return 0xFF;
    if (strcmp(t, "u16") == 0 || strcmp(t, "uint16_t") == 0 || strcmp(t, "ushort") == 0) return 0xFFFF;
    if (strcmp(t, "u32") == 0 || strcmp(t, "uint32_t") == 0 || strcmp(t, "uint") == 0) return 0xFFFFFFFF;
    return 0xFFFFFFFFFFFFFFFFULL;
}

/* ----------------------------------------------- json */

static void json_metadata(Buf *b, const Class *c) {
    if (c->nmd == 0) {
        buf_puts(b, "[]");
        return;
    }
    buf_puts(b, "[\n");
    for (size_t i = 0; i < c->nmd; i++) {
        const Metadata *m = &c->md[i];
        const char *type = m->kind == MD_CHANGE_CB ? "NetworkChangeCallback"
                         : m->kind == MD_VAR_NAMES ? "NetworkVarNames"
                                                   : "Unknown";
        buf_puts(b, "          {\n            \"name\": \"");
        json_escape(b, m->name);
        buf_puts(b, "\",\n            \"type\": \"");
        buf_puts(b, type);
        buf_puts(b, "\"");
        if (m->kind == MD_VAR_NAMES) {
            buf_puts(b, ",\n            \"type_name\": \"");
            json_escape(b, m->type_name);
            buf_puts(b, "\"");
        }
        buf_printf(b, "\n          }%s\n", i + 1 < c->nmd ? "," : "");
    }
    buf_puts(b, "        ]");
}

static int cmp_field_name(const void *a, const void *b) {
    return strcmp(((const Field *)a)->name, ((const Field *)b)->name);
}

static int cmp_member_name(const void *a, const void *b) {
    return strcmp(((const EnumMember *)a)->name, ((const EnumMember *)b)->name);
}

/* JSON is keyed alphabetically, as a map; the code formats keep declaration
   order. */
static void json_schema(Buf *b, const Schema *s) {
    buf_puts(b, "{\n  \"");
    json_escape(b, s->module);
    buf_puts(b, "\": {\n    \"classes\": {");

    for (size_t i = 0; i < s->nclasses; i++) {
        const Class *c = &s->classes[i];
        char *slug = name_slugify(c->name);
        Field *sorted = xmalloc((c->nfields ? c->nfields : 1) * sizeof(*sorted));
        memcpy(sorted, c->fields, c->nfields * sizeof(*sorted));
        qsort(sorted, c->nfields, sizeof(*sorted), cmp_field_name);

        buf_printf(b, "%s\n      \"", i ? "," : "");
        json_escape(b, slug);
        buf_puts(b, "\": {\n        \"fields\": {");
        for (size_t k = 0; k < c->nfields; k++) {
            buf_printf(b, "%s\n          \"", k ? "," : "");
            json_escape(b, sorted[k].name);
            buf_printf(b, "\": %d", sorted[k].offset);
        }
        if (c->nfields) buf_puts(b, "\n        }");
        else buf_puts(b, "}");

        buf_puts(b, ",\n        \"metadata\": ");
        json_metadata(b, c);
        buf_puts(b, ",\n        \"parent\": ");
        if (c->parent) {
            buf_puts(b, "\"");
            json_escape(b, c->parent);
            buf_puts(b, "\"");
        } else {
            buf_puts(b, "null");
        }
        buf_puts(b, "\n      }");
        free(sorted);
        free(slug);
    }
    if (s->nclasses) buf_puts(b, "\n    }");
    else buf_puts(b, "}");

    buf_puts(b, ",\n    \"enums\": {");
    for (size_t i = 0; i < s->nenums; i++) {
        const Enum *e = &s->enums[i];
        char *slug = name_slugify(e->name);
        EnumMember *sorted = xmalloc((e->nmembers ? e->nmembers : 1) * sizeof(*sorted));
        memcpy(sorted, e->members, e->nmembers * sizeof(*sorted));
        qsort(sorted, e->nmembers, sizeof(*sorted), cmp_member_name);

        buf_printf(b, "%s\n      \"", i ? "," : "");
        json_escape(b, slug);
        buf_printf(b, "\": {\n        \"alignment\": %u,\n        \"members\": {",
                   (unsigned)e->alignment);
        for (size_t k = 0; k < e->nmembers; k++) {
            buf_printf(b, "%s\n          \"", k ? "," : "");
            json_escape(b, sorted[k].name);
            buf_printf(b, "\": %lld", (long long)sorted[k].value);
        }
        if (e->nmembers) buf_puts(b, "\n        }");
        else buf_puts(b, "}");
        buf_printf(b, ",\n        \"type\": \"%s\"\n      }", json_align_type(e->alignment));
        free(sorted);
        free(slug);
    }
    if (s->nenums) buf_puts(b, "\n    }");
    else buf_puts(b, "}");

    buf_puts(b, "\n  }\n}\n");
}

/* ----------------------------------------------- enum members */

/* Enum member values, per format. Negative values are rendered differently in
   each language: `.hpp` clamps to the type max, `.cs` uses an unchecked cast,
   `.rs` uses the full two's-complement bit pattern (and `u32::MAX` for -1), and
   `.zig` wraps to the declared width. */
static void enum_members(Buf *b, const Enum *e, const char *lang, int indent) {
    int rs = strcmp(lang, "rs") == 0;
    int cs = strcmp(lang, "cs") == 0;
    u64 *used = xcalloc(e->nmembers ? e->nmembers : 1, sizeof(*used));
    size_t nused = 0;

    for (size_t i = 0; i < e->nmembers; i++) {
        if (rs) {
            int dup = 0;
            for (size_t k = 0; k < nused; k++)
                if (used[k] == (u64)e->members[i].value) dup = 1;
            if (dup) continue;
            used[nused++] = (u64)e->members[i].value;
        }
        json_indent(b, indent, 4);
        char *mn = rs ? name_rust(e->members[i].name) : xstrdup(e->members[i].name);
        buf_puts(b, mn);
        free(mn);
        buf_puts(b, " = ");

        i64 v = e->members[i].value;
        if (v >= 0) {
            buf_printf(b, "0x%llX", (unsigned long long)v);
        } else if (rs) {
            /* Wrap to the declared width. The reference emits the full i64
               two's complement here, which rustc rejects as an overflowing
               literal in a `#[repr(u32)]` enum. */
            if (v == -1) buf_printf(b, "%s::MAX", align_type(e, "rs"));
            else buf_printf(b, "0x%llX",
                            (unsigned long long)((u64)v & align_max(align_type(e, "rs"))));
        } else if (cs) {
            buf_printf(b, "unchecked((%s)%lld)", align_type(e, "cs"), (long long)v);
        } else {
            buf_printf(b, "0x%llX", (unsigned long long)align_max(align_type(e, lang)));
        }
        buf_printf(b, "%s\n", i + 1 < e->nmembers ? "," : "");
    }
    free(used);
}

/* ----------------------------------------------- code formats */

static void schema_cs(Buf *b, const Schema *s, int indent) {
    char *cls = name_pascal_of(s->module);
    buf_puts(b, "namespace CS2Dumper.Schemas {\n");
    json_indent(b, 1, indent);
    buf_printf(b, "public static class %s {\n", cls);
    free(cls);

    for (size_t i = 0; i < s->nenums; i++) {
        const Enum *e = &s->enums[i];
        const char *t = align_type(e, "cs");
        if (!t) continue;
        char *en = name_slugify(e->name);
        json_indent(b, 2, indent);
        buf_printf(b, "public enum %s : %s {\n", en, t);
        free(en);
        enum_members(b, e, "cs", 3);
        json_indent(b, 2, indent);
        buf_puts(b, "}\n");
    }

    for (size_t i = 0; i < s->nclasses; i++) {
        const Class *c = &s->classes[i];
        char *cn = name_slugify(c->name);
        json_indent(b, 2, indent);
        buf_printf(b, "public static class %s {\n", cn);
        free(cn);
        for (size_t k = 0; k < c->nfields; k++) {
            json_indent(b, 3, indent);
            buf_printf(b, "public const nint %s = 0x%X; // %s\n", c->fields[k].name,
                       (unsigned)c->fields[k].offset, c->fields[k].type_name);
        }
        json_indent(b, 2, indent);
        buf_puts(b, "}\n");
    }

    json_indent(b, 1, indent);
    buf_puts(b, "}\n}\n");
}

static void schema_hpp(Buf *b, const Schema *s, int indent) {
    buf_puts(b, "#pragma once\n\n#include <cstddef>\n#include <cstdint>\n\n");
    buf_puts(b, "namespace cs2_dumper {\n");
    json_indent(b, 1, indent);
    buf_puts(b, "namespace schemas {\n");

    char *sn = name_snake_of(s->module);
    json_indent(b, 2, indent);
    buf_printf(b, "namespace %s {\n", sn);
    free(sn);

    for (size_t i = 0; i < s->nenums; i++) {
        const Enum *e = &s->enums[i];
        const char *t = align_type(e, "hpp");
        if (!t) continue;
        char *en = name_slugify(e->name);
        json_indent(b, 3, indent);
        buf_printf(b, "enum class %s : %s {\n", en, t);
        free(en);
        enum_members(b, e, "hpp", 4);
        json_indent(b, 3, indent);
        buf_puts(b, "};\n");
    }

    for (size_t i = 0; i < s->nclasses; i++) {
        const Class *c = &s->classes[i];
        char *cn = name_slugify(c->name);
        json_indent(b, 3, indent);
        buf_printf(b, "namespace %s {\n", cn);
        free(cn);
        for (size_t k = 0; k < c->nfields; k++) {
            json_indent(b, 4, indent);
            buf_printf(b, "constexpr std::ptrdiff_t %s = 0x%X; // %s\n", c->fields[k].name,
                       (unsigned)c->fields[k].offset, c->fields[k].type_name);
        }
        json_indent(b, 3, indent);
        buf_puts(b, "}\n");
    }

    json_indent(b, 2, indent);
    buf_puts(b, "}\n");
    json_indent(b, 1, indent);
    buf_puts(b, "}\n}\n");
}

static void schema_rs(Buf *b, const Schema *s, int indent) {
    buf_puts(b, "#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case, "
                "unused)]\n\n");
    buf_puts(b, "pub mod cs2_dumper {\n");
    json_indent(b, 1, indent);
    buf_puts(b, "pub mod schemas {\n");

    char *sn = name_snake_of(s->module);
    json_indent(b, 2, indent);
    buf_printf(b, "pub mod %s {\n", sn);
    free(sn);

    for (size_t i = 0; i < s->nenums; i++) {
        const Enum *e = &s->enums[i];
        const char *t = align_type(e, "rs");
        if (!t) continue;
        json_indent(b, 3, indent);
        buf_printf(b, "#[repr(%s)]\n", t);
        char *en = name_slugify(e->name);
        json_indent(b, 3, indent);
        buf_printf(b, "pub enum %s {\n", en);
        free(en);
        enum_members(b, e, "rs", 4);
        json_indent(b, 3, indent);
        buf_puts(b, "}\n");
    }

    for (size_t i = 0; i < s->nclasses; i++) {
        const Class *c = &s->classes[i];
        char *cn = name_slugify(c->name);
        json_indent(b, 3, indent);
        buf_printf(b, "pub mod %s {\n", cn);
        free(cn);
        for (size_t k = 0; k < c->nfields; k++) {
            json_indent(b, 4, indent);
            char *fn = name_rust(c->fields[k].name);
            buf_printf(b, "pub const %s: usize = 0x%X; // %s\n", fn,
                       (unsigned)c->fields[k].offset, c->fields[k].type_name);
            free(fn);
        }
        json_indent(b, 3, indent);
        buf_puts(b, "}\n");
    }

    json_indent(b, 2, indent);
    buf_puts(b, "}\n");
    json_indent(b, 1, indent);
    buf_puts(b, "}\n}\n");
}

/* Zig enum values are written as the bit pattern of the declared type. */
static void zig_enum_members(Buf *b, const Enum *e, const char *t, int indent) {
    u64 *used = xcalloc(e->nmembers ? e->nmembers : 1, sizeof(*used));
    size_t nused = 0;

    for (size_t i = 0; i < e->nmembers; i++) {
        int dup = 0;
        for (size_t k = 0; k < nused; k++)
            if (used[k] == (u64)e->members[i].value) dup = 1;
        if (dup) continue;
        used[nused++] = (u64)e->members[i].value;

        u64 v = e->members[i].value < 0
                    ? ((u64)e->members[i].value & align_max(t))
                    : (u64)e->members[i].value;
        json_indent(b, indent, 4);
        char *id = name_zig(e->members[i].name);
        buf_printf(b, "%s = 0x%llX%s\n", id, (unsigned long long)v,
                   i + 1 < e->nmembers ? "," : "");
        free(id);
    }
    free(used);
}

static void schema_zig(Buf *b, const Schema *s, int indent) {
    buf_puts(b, "pub const cs2_dumper = struct {\n");
    json_indent(b, 1, indent);
    buf_puts(b, "pub const schemas = struct {\n");

    char *sn = name_zig_of(s->module);
    json_indent(b, 2, indent);
    buf_printf(b, "pub const %s = struct {\n", sn);
    free(sn);

    for (size_t i = 0; i < s->nenums; i++) {
        const Enum *e = &s->enums[i];
        const char *t = align_type(e, "zig");
        if (!t) continue;
        char *es = name_slugify(e->name);
        char *en = name_zig(es);
        free(es);
        json_indent(b, 3, indent);
        buf_printf(b, "pub const %s = enum(%s) {\n", en, t);
        free(en);
        zig_enum_members(b, e, t, 4);
        json_indent(b, 3, indent);
        buf_puts(b, "};\n");
    }

    for (size_t i = 0; i < s->nclasses; i++) {
        const Class *c = &s->classes[i];
        char *cs = name_slugify(c->name);
        char *cn = name_zig(cs);
        free(cs);
        json_indent(b, 3, indent);
        buf_printf(b, "pub const %s = struct {\n", cn);
        free(cn);
        for (size_t k = 0; k < c->nfields; k++) {
            char *fn = name_zig(c->fields[k].name);
            json_indent(b, 4, indent);
            buf_printf(b, "pub const %s: usize = 0x%X; // %s\n", fn,
                       (unsigned)c->fields[k].offset, c->fields[k].type_name);
            free(fn);
        }
        json_indent(b, 3, indent);
        buf_puts(b, "};\n");
    }

    json_indent(b, 2, indent);
    buf_puts(b, "};\n");
    json_indent(b, 1, indent);
    buf_puts(b, "};\n};\n");
}

void write_schema(const char *dir, const char *module, const Schema *s,
                  const FileTypes *types, int indent) {
    char *base = name_slugify(module);

    if (output_wants(types, "json")) {
        Buf b;
        buf_init(&b);
        json_schema(&b, s);
        write_file(dir, base, "json", &b);
        free(b.p);
    }
    if (output_wants(types, "cs")) {
        Buf b;
        buf_init(&b);
        output_begin(&b);
        schema_cs(&b, s, indent);
        write_file(dir, base, "cs", &b);
        free(b.p);
    }
    if (output_wants(types, "hpp")) {
        Buf b;
        buf_init(&b);
        output_begin(&b);
        schema_hpp(&b, s, indent);
        write_file(dir, base, "hpp", &b);
        free(b.p);
    }
    if (output_wants(types, "rs")) {
        Buf b;
        buf_init(&b);
        output_begin(&b);
        schema_rs(&b, s, indent);
        write_file(dir, base, "rs", &b);
        free(b.p);
    }
    if (output_wants(types, "zig")) {
        Buf b;
        buf_init(&b);
        output_begin(&b);
        schema_zig(&b, s, indent);
        write_file(dir, base, "zig", &b);
        free(b.p);
    }
    free(base);
}
