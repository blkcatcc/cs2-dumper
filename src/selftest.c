#include "dumper.h"

#include <sys/stat.h>
#include <unistd.h>

/* Asserts the emitters produce exactly the expected bytes for a small fixture.
   The live path needs the game; this keeps the output format under test. */

static int failures = 0;
static const char *g_dir = "selftest-out";

static const char *path_of(const char *base, const char *ext) {
    static char buf[512];
    snprintf(buf, sizeof(buf), "%s/%s.%s", g_dir, base, ext);
    return buf;
}

static char *slurp(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long n = ftell(f);
    rewind(f);
    char *buf = xmalloc((size_t)n + 1);
    size_t got = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[got] = 0;
    return buf;
}

static void expect(const char *label, const char *path, const char *want) {
    char *got = slurp(path);
    if (!got) {
        printf("FAIL %-24s (no file)\n", label);
        failures++;
        return;
    }
    if (strcmp(got, want) != 0) {
        printf("FAIL %-24s\n--- want ---\n%s\n--- got ---\n%s\n", label, want, got);
        failures++;
    } else {
        printf("ok   %s\n", label);
    }
    free(got);
}

/* Regression: the schema map used to be allocated for a fixed 8 entries and
   written one per type scope, so a real game (which has far more scopes) wrote
   past the end. Drives the growth path directly. */
static void test_schema_map_growth(void) {
    enum { N = 40 };
    SchemaMap map;
    size_t cap = 8;
    map.v = xcalloc(cap, sizeof(*map.v));
    map.n = 0;

    for (int i = 0; i < N; i++) {
        if (map.n == cap) {
            cap *= 2;
            map.v = xrealloc(map.v, cap * sizeof(*map.v));
            memset(map.v + map.n, 0, (cap - map.n) * sizeof(*map.v));
        }
        Schema *sc = &map.v[map.n];
        char name[32];
        snprintf(name, sizeof(name), "module_%d.so", i);
        sc->module = xstrdup(name);
        sc->classes = xcalloc(1, sizeof(*sc->classes));
        sc->nclasses = 0;
        sc->enums = xcalloc(1, sizeof(*sc->enums));
        sc->nenums = 0;
        map.n++;
    }

    if (map.n != N) {
        printf("FAIL schema map growth (%zu != %d)\n", map.n, N);
        failures++;
    } else {
        printf("ok   schema map growth (%zu entries)\n", map.n);
    }

    for (size_t i = 0; i < map.n; i++) {
        free(map.v[i].module);
        free(map.v[i].classes);
        free(map.v[i].enums);
    }
    free(map.v);
}

int selftest_main(void) {
    const char *dir = g_dir;
    mkdir(dir, 0755);

    test_schema_map_growth();

    g_banner = "// build 12345\n\n";

    FileTypes all = { (const char *[]){ "cs", "hpp", "json", "rs", "zig" }, 5 };

    NameVal offs[] = {
        { "dwAlpha", 0x10 },
        { "dwBeta", 0x20 },
    };
    NVGroup grp = { "libclient.so", offs, 2 };
    NVMap map = { &grp, 1 };
    write_nv_map(dir, "offsets", &map, &all, 4);

    expect("offsets.hpp", path_of("offsets", "hpp"),
           "// build 12345\n"
           "\n"
           "#pragma once\n"
           "\n"
           "#include <cstddef>\n"
           "#include <cstdint>\n"
           "\n"
           "namespace cs2_dumper {\n"
           "    namespace offsets {\n"
           "        namespace libclient_so {\n"
           "            constexpr std::ptrdiff_t dwAlpha = 0x10;\n"
           "            constexpr std::ptrdiff_t dwBeta = 0x20;\n"
           "        }\n"
           "    }\n"
           "}\n");

    expect("offsets.zig", path_of("offsets", "zig"),
           "// build 12345\n"
           "\n"
           "pub const cs2_dumper = struct {\n"
           "    pub const offsets = struct {\n"
           "        pub const libclient_so = struct {\n"
           "            pub const dwAlpha: usize = 0x10;\n"
           "            pub const dwBeta: usize = 0x20;\n"
           "        };\n"
           "    };\n"
           "};\n");

    expect("offsets.json", path_of("offsets", "json"),
           "{\n"
           "  \"libclient.so\": {\n"
           "    \"dwAlpha\": 16,\n"
           "    \"dwBeta\": 32\n"
           "  }\n"
           "}\n");

    NameVal btns[] = { { "use", 0x30 }, { "zoom", 0x40 } };
    NVGroup bgrp = { "libclient.so", btns, 2 };
    NVMap bmap = { &bgrp, 1 };
    write_nv_map(dir, "buttons", &bmap, &all, 4);

    expect("buttons.hpp", path_of("buttons", "hpp"),
           "// build 12345\n"
           "\n"
           "#pragma once\n"
           "\n"
           "#include <cstddef>\n"
           "#include <cstdint>\n"
           "\n"
           "namespace cs2_dumper {\n"
           "    namespace buttons {\n"
           "        constexpr std::ptrdiff_t use = 0x30;\n"
           "        constexpr std::ptrdiff_t zoom = 0x40;\n"
           "    }\n"
           "}\n");

    expect("buttons.rs", path_of("buttons", "rs"),
           "// build 12345\n"
           "\n"
           "#![allow(non_upper_case_globals, unused)]\n"
           "\n"
           "pub mod cs2_dumper {\n"
           "    pub mod buttons {\n"
           "        pub const r#use: usize = 0x30;\n"
           "        pub const zoom: usize = 0x40;\n"
           "    }\n"
           "}\n");

    /* One class with a field, one enum with two members. */
    Field fields[] = { { "m_Handle", "CHandle< CBaseEntity >", 0x30 } };
    Class classes[] = { { "CAttributeList", "CBaseEntity", fields, 1, NULL, 0 } };
    EnumMember members[] = { { "kNone", 0 }, { "kSome", 1 } };
    Enum enums[] = { { "EFoo_t", 4, 2, members, 2 } };
    Schema schema = { "libclient.so", classes, 1, enums, 1 };
    write_schema(dir, "libclient.so", &schema, &all, 4);

    expect("schema.hpp", path_of("libclient_so", "hpp"),
           "// build 12345\n"
           "\n"
           "#pragma once\n"
           "\n"
           "#include <cstddef>\n"
           "#include <cstdint>\n"
           "\n"
           "namespace cs2_dumper {\n"
           "    namespace schemas {\n"
           "        namespace libclient_so {\n"
           "            enum class EFoo_t : uint32_t {\n"
           "                kNone = 0x0,\n"
           "                kSome = 0x1\n"
           "            };\n"
           "            namespace CAttributeList {\n"
           "                constexpr std::ptrdiff_t m_Handle = 0x30; // CHandle< CBaseEntity >\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "}\n");

    expect("schema.json", path_of("libclient_so", "json"),
           "{\n"
           "  \"libclient.so\": {\n"
           "    \"classes\": {\n"
           "      \"CAttributeList\": {\n"
           "        \"fields\": {\n"
           "          \"m_Handle\": 48\n"
           "        },\n"
           "        \"metadata\": [],\n"
           "        \"parent\": \"CBaseEntity\"\n"
           "      }\n"
           "    },\n"
           "    \"enums\": {\n"
           "      \"EFoo_t\": {\n"
           "        \"alignment\": 4,\n"
           "        \"members\": {\n"
           "          \"kNone\": 0,\n"
           "          \"kSome\": 1\n"
           "        },\n"
           "        \"type\": \"uint32\"\n"
           "      }\n"
           "    }\n"
           "  }\n"
           "}\n");

    /* Empty fields/metadata plus a null parent: the containers must close once. */
    Class empty_classes[] = {
        { "CEmpty", NULL, NULL, 0, NULL, 0 },
        { "CWithFields", "CEmpty", fields, 1, NULL, 0 },
    };
    Schema empty_schema = { "libclient.so", empty_classes, 2, NULL, 0 };
    write_schema(dir, "libclient.so", &empty_schema, &all, 4);

    expect("empty schema.json", path_of("libclient_so", "json"),
           "{\n"
           "  \"libclient.so\": {\n"
           "    \"classes\": {\n"
           "      \"CEmpty\": {\n"
           "        \"fields\": {},\n"
           "        \"metadata\": [],\n"
           "        \"parent\": null\n"
           "      },\n"
           "      \"CWithFields\": {\n"
           "        \"fields\": {\n"
           "          \"m_Handle\": 48\n"
           "        },\n"
           "        \"metadata\": [],\n"
           "        \"parent\": \"CEmpty\"\n"
           "      }\n"
           "    },\n"
           "    \"enums\": {}\n"
           "  }\n"
           "}\n");

    /* Rust keyword names must be escaped or the output will not compile. */
    EnumMember kw_members[] = { { "Self", 0 }, { "type", 1 }, { "Ok", 2 } };
    Enum kw_enum[] = { { "EKind", 4, 3, kw_members, 3 } };
    Field kw_fields[] = { { "type", "int32_t", 0x10 } };
    Class kw_classes[] = { { "CKind", NULL, kw_fields, 1, NULL, 0 } };
    Schema kw_schema = { "libclient.so", kw_classes, 1, kw_enum, 1 };
    write_schema(dir, "libclient.so", &kw_schema, &all, 4);

    expect("rust keyword escaping", path_of("libclient_so", "rs"),
           "// build 12345\n"
           "\n"
           "#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case, unused)]\n"
           "\n"
           "pub mod cs2_dumper {\n"
           "    pub mod schemas {\n"
           "        pub mod libclient_so {\n"
           "            #[repr(u32)]\n"
           "            pub enum EKind {\n"
           "                Self_ = 0x0,\n"
           "                r#type = 0x1,\n"
           "                Ok = 0x2\n"
           "            }\n"
           "            pub mod CKind {\n"
           "                pub const r#type: usize = 0x10; // int32_t\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "}\n");

    /* Negative enum values render differently per format. */
    EnumMember neg_members[] = { { "kInvalid", -1 }, { "kPromoted", -2 } };
    Enum neg_enum[] = { { "ESlot", 4, 2, neg_members, 2 } };
    Schema neg_schema = { "libclient.so", NULL, 0, neg_enum, 1 };
    write_schema(dir, "libclient.so", &neg_schema, &all, 4);

    expect("negative enum .hpp", path_of("libclient_so", "hpp"),
           "// build 12345\n"
           "\n"
           "#pragma once\n"
           "\n"
           "#include <cstddef>\n"
           "#include <cstdint>\n"
           "\n"
           "namespace cs2_dumper {\n"
           "    namespace schemas {\n"
           "        namespace libclient_so {\n"
           "            enum class ESlot : uint32_t {\n"
           "                kInvalid = 0xFFFFFFFF,\n"
           "                kPromoted = 0xFFFFFFFF\n"
           "            };\n"
           "        }\n"
           "    }\n"
           "}\n");

    expect("negative enum .cs", path_of("libclient_so", "cs"),
           "// build 12345\n"
           "\n"
           "namespace CS2Dumper.Schemas {\n"
           "    public static class LibclientSo {\n"
           "        public enum ESlot : uint {\n"
           "            kInvalid = unchecked((uint)-1),\n"
           "            kPromoted = unchecked((uint)-2)\n"
           "        }\n"
           "    }\n"
           "}\n");

    expect("negative enum .rs", path_of("libclient_so", "rs"),
           "// build 12345\n"
           "\n"
           "#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case, unused)]\n"
           "\n"
           "pub mod cs2_dumper {\n"
           "    pub mod schemas {\n"
           "        pub mod libclient_so {\n"
           "            #[repr(u32)]\n"
           "            pub enum ESlot {\n"
           "                kInvalid = u32::MAX,\n"
           "                kPromoted = 0xFFFFFFFE\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "}\n");

    printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
