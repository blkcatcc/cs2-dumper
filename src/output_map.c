#include "dumper.h"

/* Offsets, interfaces and buttons. All three share a shape: a namespace named
   after the file, then either the values directly (buttons) or one block per
   module (offsets, interfaces). */

static void json_nv(Buf *b, const NVMap *map) {
    buf_puts(b, "{\n");
    for (size_t g = 0; g < map->n; g++) {
        const NVGroup *grp = &map->v[g];
        buf_puts(b, "  \"");
        json_escape(b, grp->module);
        buf_puts(b, "\": {\n");
        for (size_t i = 0; i < grp->n; i++) {
            buf_puts(b, "    \"");
            json_escape(b, grp->v[i].name);
            buf_printf(b, "\": %llu%s\n", (unsigned long long)grp->v[i].value,
                       i + 1 < grp->n ? "," : "");
        }
        buf_printf(b, "  }%s\n", g + 1 < map->n ? "," : "");
    }
    buf_puts(b, "}\n");
}

static const char *nv_cs_namespace(const char *base_name) {
    if (strcmp(base_name, "interfaces") == 0) return "CS2Dumper.Interfaces";
    if (strcmp(base_name, "offsets") == 0) return "CS2Dumper.Offsets";
    return "CS2Dumper";
}

static void nv_cs(const char *dir, const char *base_name, const NVMap *map, int nest,
                  int indent) {
    Buf b;
    buf_init(&b);
    output_begin(&b);
    buf_printf(&b, "namespace %s {\n", nv_cs_namespace(base_name));
    for (size_t g = 0; g < map->n; g++) {
        const NVGroup *grp = &map->v[g];
        char *cls = nest ? name_pascal_of(grp->module) : xstrdup("Buttons");
        json_indent(&b, 1, indent);
        buf_printf(&b, "public static class %s {\n", cls);
        free(cls);
        for (size_t i = 0; i < grp->n; i++) {
            json_indent(&b, 2, indent);
            buf_printf(&b, "public const nint %s = 0x%X;\n", grp->v[i].name,
                       (unsigned)grp->v[i].value);
        }
        json_indent(&b, 1, indent);
        buf_puts(&b, "}\n");
    }
    buf_puts(&b, "}\n");
    write_file(dir, base_name, "cs", &b);
    free(b.p);
}

static void nv_hpp(const char *dir, const char *base_name, const NVMap *map, int nest,
                   int indent) {
    Buf b;
    buf_init(&b);
    output_begin(&b);
    buf_puts(&b, "#pragma once\n\n#include <cstddef>\n#include <cstdint>\n\n");
    buf_puts(&b, "namespace cs2_dumper {\n");
    json_indent(&b, 1, indent);
    buf_printf(&b, "namespace %s {\n", base_name);
    int base = 2;
    for (size_t g = 0; g < map->n; g++) {
        const NVGroup *grp = &map->v[g];
        if (nest) {
            char *sn = name_snake_of(grp->module);
            json_indent(&b, base, indent);
            buf_printf(&b, "namespace %s {\n", sn);
            free(sn);
        }
        for (size_t i = 0; i < grp->n; i++) {
            json_indent(&b, base + (nest ? 1 : 0), indent);
            buf_printf(&b, "constexpr std::ptrdiff_t %s = 0x%X;\n", grp->v[i].name,
                       (unsigned)grp->v[i].value);
        }
        if (nest) {
            json_indent(&b, base, indent);
            buf_puts(&b, "}\n");
        }
    }
    json_indent(&b, 1, indent);
    buf_puts(&b, "}\n}\n");
    write_file(dir, base_name, "hpp", &b);
    free(b.p);
}

static void nv_rs(const char *dir, const char *base_name, const NVMap *map, int nest,
                  int indent) {
    Buf b;
    buf_init(&b);
    output_begin(&b);
    buf_puts(&b, "#![allow(non_upper_case_globals, unused)]\n\n");
    buf_puts(&b, "pub mod cs2_dumper {\n");
    json_indent(&b, 1, indent);
    buf_printf(&b, "pub mod %s {\n", base_name);
    int base = 2;
    for (size_t g = 0; g < map->n; g++) {
        const NVGroup *grp = &map->v[g];
        if (nest) {
            char *sn = name_snake_of(grp->module);
            json_indent(&b, base, indent);
            buf_printf(&b, "pub mod %s {\n", sn);
            free(sn);
        }
        for (size_t i = 0; i < grp->n; i++) {
            char *name = name_rust(grp->v[i].name);
            json_indent(&b, base + (nest ? 1 : 0), indent);
            buf_printf(&b, "pub const %s: usize = 0x%X;\n", name, (unsigned)grp->v[i].value);
            free(name);
        }
        if (nest) {
            json_indent(&b, base, indent);
            buf_puts(&b, "}\n");
        }
    }
    json_indent(&b, 1, indent);
    buf_puts(&b, "}\n}\n");
    write_file(dir, base_name, "rs", &b);
    free(b.p);
}

static void nv_zig(const char *dir, const char *base_name, const NVMap *map, int nest,
                   int indent) {
    Buf b;
    buf_init(&b);
    output_begin(&b);
    buf_puts(&b, "pub const cs2_dumper = struct {\n");
    json_indent(&b, 1, indent);
    buf_printf(&b, "pub const %s = struct {\n", base_name);
    int base = 2;
    for (size_t g = 0; g < map->n; g++) {
        const NVGroup *grp = &map->v[g];
        if (nest) {
            char *sn = name_zig_of(grp->module);
            json_indent(&b, base, indent);
            buf_printf(&b, "pub const %s = struct {\n", sn);
            free(sn);
        }
        for (size_t i = 0; i < grp->n; i++) {
            char *id = name_zig(grp->v[i].name);
            json_indent(&b, base + (nest ? 1 : 0), indent);
            buf_printf(&b, "pub const %s: usize = 0x%X;\n", id, (unsigned)grp->v[i].value);
            free(id);
        }
        if (nest) {
            json_indent(&b, base, indent);
            buf_puts(&b, "};\n");
        }
    }
    json_indent(&b, 1, indent);
    buf_puts(&b, "};\n};\n");
    write_file(dir, base_name, "zig", &b);
    free(b.p);
}

void write_nv_map(const char *dir, const char *base_name, const NVMap *map,
                  const FileTypes *types, int indent) {
    int nest = strcmp(base_name, "buttons") != 0;

    if (output_wants(types, "json")) {
        Buf b;
        buf_init(&b);
        json_nv(&b, map);
        write_file(dir, base_name, "json", &b);
        free(b.p);
    }
    if (output_wants(types, "cs")) nv_cs(dir, base_name, map, nest, indent);
    if (output_wants(types, "hpp")) nv_hpp(dir, base_name, map, nest, indent);
    if (output_wants(types, "rs")) nv_rs(dir, base_name, map, nest, indent);
    if (output_wants(types, "zig")) nv_zig(dir, base_name, map, nest, indent);
}
