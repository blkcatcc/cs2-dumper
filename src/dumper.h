#ifndef DUMPER_H
#define DUMPER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;

/* ---------------------------------------------------------------- util */

void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t sz);
void *xrealloc(void *p, size_t n);
char *xstrdup(const char *s);

typedef struct {
    char *p;
    size_t len, cap;
} Buf;

void buf_init(Buf *b);
void buf_add(Buf *b, const char *s, size_t n);
void buf_puts(Buf *b, const char *s);
void buf_printf(Buf *b, const char *fmt, ...);

typedef struct {
    char **v;
    size_t n, cap;
} Strs;

void strs_push(Strs *s, const char *str);
int strs_find(const Strs *s, const char *str);
void strs_free(Strs *s);

/* -------------------------------------------------------------- strings */

char *str_prepend_once(const char *dst, const char *src);
char *str_lower(const char *s);
int str_starts(const char *a, const char *prefix);

/* ---------------------------------------------------------- json parser */

typedef enum { J_NULL, J_BOOL, J_NUM, J_STR, J_ARR, J_OBJ } JKind;

typedef struct Json {
    JKind kind;
    int boolean;
    double num;
    char *str;
    struct Json **items;
    char **keys;
    size_t n;
} Json;

Json *json_parse(const char *text, size_t len);
void json_free(Json *j);
Json *json_get(const Json *o, const char *key);
const char *json_str(const Json *j, const char *key, const char *def);
double json_num(const Json *j, const char *key, double def);

/* --------------------------------------------------------------- config */

typedef enum { OP_ADD, OP_SUB, OP_RIP, OP_READ, OP_SLICE } OpKind;

typedef struct {
    OpKind kind;
    u64 value;
    u64 offset, len;
    u64 start, end;
} Op;

typedef struct {
    char *name;
    char *pattern;
    Op *ops;
    size_t nops;
} Sig;

typedef struct {
    char *module;
    Sig *sigs;
    size_t nsigs;
} SigGroup;

typedef struct {
    char *executable;
    SigGroup *groups;
    size_t ngroups;
} Config;

Config *config_load(void);

/* ------------------------------------------------------------- patterns */

typedef struct {
    u8 *bytes;
    u8 *mask;
    size_t n;
} Pattern;

int pattern_parse(Pattern *pat, const char *text);
void pattern_free(Pattern *pat);
int pattern_scan(const Pattern *pat, const u8 *buf, size_t len, size_t *off);

/* ---------------------------------------------------------------- image */

typedef struct {
    u64 off, vaddr, filesz, memsz;
} Seg;

typedef struct {
    u8 *data;
    size_t len;
    u64 base;
    Seg *segs;
    size_t nsegs;
    char *path;
} Image;

int image_load(Image *img, const char *path);
void image_free(Image *img);
u64 image_export(const Image *img, const char *name);
u64 image_off_to_vaddr(const Image *img, size_t off);
i64 image_vaddr_to_off(const Image *img, u64 vaddr);

/* --------------------------------------------------------------- memory */

typedef struct {
    int fd;
    pid_t pid;
} Mem;

int mem_open(Mem *m, pid_t pid);
void mem_close(Mem *m);
int mem_read(const Mem *m, u64 addr, void *dst, size_t n);
u64 mem_read_u64(const Mem *m, u64 addr);
u32 mem_read_u32(const Mem *m, u64 addr);
char *mem_read_str(const Mem *m, u64 addr, size_t max);

/* --------------------------------------------------------------- module */

typedef struct {
    char *name;
    u64 base, size;
} Module;

typedef struct {
    Module *v;
    size_t n;
} Modules;

pid_t proc_find(const char *name);
int modules_enum(pid_t pid, Modules *out);
void modules_free(Modules *m);
const Module *modules_get(const Modules *m, const char *name);

u64 resolve_rip(const Mem *mem, u64 addr, u64 offset, u64 len);

/* --------------------------------------------------------- analysis types */

typedef struct {
    char *name;
    u64 value;
} NameVal;

typedef struct {
    char *module;
    NameVal *v;
    size_t n;
} NVGroup;

typedef struct {
    NVGroup *v;
    size_t n;
} NVMap;

typedef struct {
    char *name;
    char *type_name;
    i32 offset;
} Field;

typedef enum { MD_UNKNOWN, MD_CHANGE_CB, MD_VAR_NAMES } MdKind;

typedef struct {
    MdKind kind;
    char *name;
    char *type_name;
} Metadata;

typedef struct {
    char *name;
    char *parent;
    Field *fields;
    size_t nfields;
    Metadata *md;
    size_t nmd;
} Class;

typedef struct {
    char *name;
    i64 value;
} EnumMember;

typedef struct {
    char *name;
    u8 alignment;
    u16 size;
    EnumMember *members;
    size_t nmembers;
} Enum;

typedef struct {
    char *module;
    Class *classes;
    size_t nclasses;
    Enum *enums;
    size_t nenums;
} Schema;

typedef struct {
    Schema *v;
    size_t n;
} SchemaMap;

typedef struct {
    u64 offset_to_top;
    u64 primary;
    u64 secondary_tables;
    u64 *slots;
    size_t nslots;
} Vtable;

typedef struct {
    char *name;
    Vtable vt;
} VtEntry;

typedef struct {
    char *module;
    VtEntry *v;
    size_t n;
} VtGroup;

typedef struct {
    VtGroup *v;
    size_t n;
} VtMap;

/* -------------------------------------------------------------- analysis */

NVMap analyze_offsets(const Mem *mem, const Modules *mods, const Config *cfg);
NVMap analyze_buttons(const Mem *mem, const Modules *mods);
NVMap analyze_interfaces(const Mem *mem, const Modules *mods);
SchemaMap analyze_schemas(const Mem *mem, const Modules *mods);
VtMap analyze_vtables(const Mem *mem, const Modules *mods);
u32 analyze_build_number(const Mem *mem, const Modules *mods, const NVMap *offsets);

/* Shared between the scan and schema passes. */
u8 *read_module(const Mem *mem, const Module *m, size_t *out_len);
void nv_push(NVMap *map, const char *module, const NameVal *vals, size_t n);

/* --------------------------------------------------------------- output */

typedef struct {
    const char **v;
    size_t n;
} FileTypes;

extern const char *g_banner; /* build-number header, or "" */

void output_begin(Buf *b);
int output_wants(const FileTypes *t, const char *ext);

void write_nv_map(const char *dir, const char *base_name, const NVMap *map,
                  const FileTypes *types, int indent);
void write_schema(const char *dir, const char *module, const Schema *s,
                  const FileTypes *types, int indent);
void write_vtables_json(const char *dir, const VtMap *map);
void write_info_json(const char *dir, u32 build_number);

void json_escape(Buf *b, const char *s);
void json_indent(Buf *b, int level, int size);

char *name_slugify(const char *s);
char *name_snake(const char *s);
char *name_pascal(const char *s);
char *name_zig(const char *s);
char *name_rust(const char *s);
char *name_snake_of(const char *s);
char *name_pascal_of(const char *s);
char *name_zig_of(const char *s);
void write_file(const char *dir, const char *base, const char *ext, const Buf *b);

#endif
