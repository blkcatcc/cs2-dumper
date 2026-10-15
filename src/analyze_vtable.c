#include "dumper.h"

#include <ctype.h>

/* Modules scanned for class vtables. */
static const char *VT_MODULES[] = {
    "libclient.so",      "libserver.so",       "libengine2.so",   "libpanorama.so",
    "libschemasystem.so","libnetworksystem.so","libscenesystem.so","libparticles.so",
    "libsoundsystem.so", "libanimationsystem.so","libmaterialsystem2.so",
    "libfilesystem_stdio.so",
};
#define VT_NMODULES (sizeof(VT_MODULES) / sizeof(VT_MODULES[0]))

/* A table ends at the next validated header or after this many non-code
   qwords (weak/NULL slots exist in the wild). */
#define MAX_SLOTS 4096
#define TRAILING_MISS_LIMIT 3

static u64 rd64(const u8 *b, size_t off) {
    u64 v;
    memcpy(&v, b + off, 8);
    return v;
}

/* ------------------------------------------------------------ mangled */

static int mangled_char(u8 c) {
    return isalnum(c) || c == '_' || c == '$' || c == '<' || c == '>';
}

/* Accepts Itanium `<len><id>` chunks, optionally wrapped in `N...E` with
   `I...E` template arguments. */
static int looks_like_mangled(const char *s) {
    size_t n = strlen(s);
    if (n == 0 || n > 512) return 0;
    for (size_t i = 0; i < n; i++)
        if (!mangled_char((u8)s[i])) return 0;

    size_t i = 0;
    if (s[0] == 'N') {
        i = 1;
        while (i < n && (s[i] == 'V' || s[i] == 'K' || s[i] == 'r')) i++;
        int depth = 1;
        while (i < n) {
            char c = s[i];
            if (c == 'E') {
                depth--;
                i++;
                if (depth == 0) return i == n;
            } else if (c == 'I') {
                depth++;
                i++;
            } else if (isdigit((u8)c)) {
                size_t len = 0;
                while (i < n && isdigit((u8)s[i])) len = len * 10 + (size_t)(s[i++] - '0');
                if (len == 0 || i + len > n) return 0;
                for (size_t k = i; k < i + len; k++)
                    if (!mangled_char((u8)s[k])) return 0;
                i += len;
            } else {
                return 0;
            }
        }
        return 0;
    }

    while (i < n) {
        if (!isdigit((u8)s[i])) return 0;
        size_t len = 0;
        while (i < n && isdigit((u8)s[i])) len = len * 10 + (size_t)(s[i++] - '0');
        if (len == 0 || i + len > n) return 0;
        for (size_t k = i; k < i + len; k++)
            if (!mangled_char((u8)s[k])) return 0;
        i += len;
    }
    return 1;
}

#define MAX_PARTS 64

/* Demangles into `ns::Class` / `Class` / `ns::Outer::Inner`. */
static char *demangle(const char *mangled) {
    size_t n = strlen(mangled);
    char *parts[MAX_PARTS];
    size_t nparts = 0;
    size_t i = 0;

    if (n && mangled[0] == 'N') {
        i = 1;
        while (i < n && (mangled[i] == 'V' || mangled[i] == 'K' || mangled[i] == 'r')) i++;
        while (i < n) {
            if (mangled[i] == 'E') break;
            if (mangled[i] == 'I') {
                size_t start = i;
                int depth = 1;
                i++;
                while (i < n && depth > 0) {
                    if (mangled[i] == 'I') depth++;
                    else if (mangled[i] == 'E') depth--;
                    i++;
                }
                if (nparts == 0) continue;
                /* Append the raw template args to the last component. */
                size_t raw_len = i >= start + 2 ? (i - 1) - (start + 1) : 0;
                char *prev = parts[nparts - 1];
                Buf b;
                buf_init(&b);
                buf_puts(&b, prev);
                buf_puts(&b, "<");
                buf_add(&b, mangled + start + 1, raw_len);
                buf_puts(&b, ">");
                free(prev);
                parts[nparts - 1] = b.p;
                continue;
            }
            size_t ds = i;
            while (i < n && isdigit((u8)mangled[i])) i++;
            if (i == ds) return xstrdup(mangled);
            size_t len = 0;
            for (size_t k = ds; k < i; k++) len = len * 10 + (size_t)(mangled[k] - '0');
            if (i + len > n || nparts == MAX_PARTS) return xstrdup(mangled);
            char *comp = xmalloc(len + 1);
            memcpy(comp, mangled + i, len);
            comp[len] = 0;
            parts[nparts++] = comp;
            i += len;
        }
    } else {
        while (i < n) {
            size_t ds = i;
            while (i < n && isdigit((u8)mangled[i])) i++;
            if (i == ds) return xstrdup(mangled);
            size_t len = 0;
            for (size_t k = ds; k < i; k++) len = len * 10 + (size_t)(mangled[k] - '0');
            if (i + len > n || nparts == MAX_PARTS) return xstrdup(mangled);
            char *comp = xmalloc(len + 1);
            memcpy(comp, mangled + i, len);
            comp[len] = 0;
            parts[nparts++] = comp;
            i += len;
        }
    }

    if (nparts == 0) return xstrdup(mangled);

    Buf out;
    buf_init(&out);
    for (size_t k = 0; k < nparts; k++) {
        if (k) buf_puts(&out, "::");
        buf_puts(&out, parts[k]);
        free(parts[k]);
    }
    return out.p;
}


/* ------------------------------------------------------------- scan */

static char *cstr_at(const u8 *image, size_t len, size_t off, size_t max) {
    if (off >= len) return NULL;
    size_t limit = off + max < len ? off + max : len;
    size_t n = 0;
    while (off + n < limit && image[off + n]) n++;
    char *s = xmalloc(n + 1);
    memcpy(s, image + off, n);
    s[n] = 0;
    return s;
}

/* Zero-terminated typeinfo name for the typeinfo object at `ti`. */
static char *typeinfo_name(const u8 *image, size_t len, u64 base, u64 ti) {
    if (ti < base) return NULL;
    size_t tio = (size_t)(ti - base);
    if (tio + 16 > len) return NULL;
    u64 name_ptr = rd64(image, tio + 8);
    if (name_ptr < base || name_ptr >= base + len) return NULL;
    return cstr_at(image, len, (size_t)(name_ptr - base), 512);
}

/* Candidate header at `off`: [offset-to-top][typeinfo], and the typeinfo's
   name must validate as a mangled name. That name check is what keeps an
   interior NULL slot followed by a code pointer from being mistaken for the
   next table's header. */
static int is_vtable_header(const u8 *image, size_t len, u64 base, size_t off) {
    if (off + 16 > len) return 0;
    i64 ott = (i64)rd64(image, off);
    if (ott > 0 || ott < -0x10000) return 0;
    u64 ti = rd64(image, off + 8);
    if (ti < base + 0x1000 || ti >= base + len) return 0;

    char *name = typeinfo_name(image, len, base, ti);
    if (!name) return 0;
    int ok = looks_like_mangled(name);
    free(name);
    return ok;
}

typedef struct {
    char *name;
    u64 primary;
    i64 ott;
    u64 secondary;
    u64 *slots;
    size_t nslots;
} VtClass;

static VtClass *vt_find(VtClass *v, size_t n, const char *name) {
    for (size_t i = 0; i < n; i++)
        if (strcmp(v[i].name, name) == 0) return &v[i];
    return NULL;
}

static int cmp_vtclass(const void *a, const void *b) {
    return strcmp(((const VtClass *)a)->name, ((const VtClass *)b)->name);
}

/* Scans one module image for vtable structures. */
static void scan_vtables(const u8 *image, size_t len, u64 base, VtClass **out, size_t *out_n) {
    size_t cap = 256, n = 0;
    VtClass *v = xcalloc(cap, sizeof(*v));

    size_t p = 0;
    while (p + 32 <= len) {
        i64 ott = (i64)rd64(image, p);
        if (ott > 0 || ott < -0x10000) {
            p += 8;
            continue;
        }
        u64 ti = rd64(image, p + 8);
        if (ti < base + 0x1000 || ti >= base + len) {
            p += 8;
            continue;
        }

        char *raw = typeinfo_name(image, len, base, ti);
        if (!raw || !looks_like_mangled(raw)) {
            free(raw);
            p += 8;
            continue;
        }
        char *name = demangle(raw);
        free(raw);

        size_t slot_cap = 64, nslots = 0;
        u64 *slots = xmalloc(slot_cap * sizeof(*slots));
        size_t misses = 0;
        size_t q = p + 16;
        while (q + 8 <= len && nslots < MAX_SLOTS) {
            if (is_vtable_header(image, len, base, q)) break;
            u64 fnptr = rd64(image, q);
            if (fnptr >= base && fnptr < base + len && (fnptr & 0xF) == 0) {
                if (nslots == slot_cap) {
                    slot_cap *= 2;
                    slots = xrealloc(slots, slot_cap * sizeof(*slots));
                }
                slots[nslots++] = fnptr - base;
                misses = 0;
            } else {
                if (++misses >= TRAILING_MISS_LIMIT) break;
            }
            q += 8;
        }

        if (nslots == 0) {
            free(slots);
            free(name);
            p += 8;
            continue;
        }

        VtClass *ex = vt_find(v, n, name);
        if (ex) {
            /* Keep the offset-to-top == 0 (complete object) table as primary. */
            ex->secondary++;
            if (ott == 0 && ex->ott != 0) {
                free(ex->slots);
                ex->primary = p + 16;
                ex->ott = ott;
                ex->slots = slots;
                ex->nslots = nslots;
            } else {
                free(slots);
            }
            free(name);
        } else {
            if (n == cap) {
                cap *= 2;
                v = xrealloc(v, cap * sizeof(*v));
            }
            VtClass *c = &v[n++];
            c->name = name;
            c->primary = p + 16;
            c->ott = ott;
            c->secondary = 0;
            c->slots = slots;
            c->nslots = nslots;
        }

        p = q; /* entries are part of the collected table */
    }

    qsort(v, n, sizeof(*v), cmp_vtclass);
    *out = v;
    *out_n = n;
}

/* Counts the vtables in a module image read from disk. Used by --selfcheck. */
size_t vtables_count_image(const u8 *image, size_t len, u64 base) {
    VtClass *classes = NULL;
    size_t n = 0;
    scan_vtables(image, len, base, &classes, &n);
    for (size_t i = 0; i < n; i++) {
        free(classes[i].name);
        free(classes[i].slots);
    }
    free(classes);
    return n;
}

VtMap analyze_vtables(const Mem *mem, const Modules *mods) {
    VtMap map;
    map.v = xcalloc(VT_NMODULES, sizeof(*map.v));
    map.n = 0;

    for (size_t i = 0; i < VT_NMODULES; i++) {
        const Module *mod = modules_get(mods, VT_MODULES[i]);
        if (!mod) {
            fprintf(stderr, "warning: skipping vtables for %s: module not found\n",
                    VT_MODULES[i]);
            continue;
        }

        size_t len = 0;
        u8 *buf = read_module(mem, mod, &len);
        if (!buf) {
            fprintf(stderr, "warning: skipping vtables for %s: unable to read module\n",
                    VT_MODULES[i]);
            continue;
        }

        VtClass *classes = NULL;
        size_t nclasses = 0;
        scan_vtables(buf, len, mod->base, &classes, &nclasses);
        free(buf);

        VtGroup *g = &map.v[map.n++];
        g->module = xstrdup(VT_MODULES[i]);
        g->v = xcalloc(nclasses ? nclasses : 1, sizeof(*g->v));
        g->n = nclasses;
        for (size_t k = 0; k < nclasses; k++) {
            g->v[k].name = classes[k].name;
            g->v[k].vt.offset_to_top = (u64)classes[k].ott;
            g->v[k].vt.primary = classes[k].primary;
            g->v[k].vt.secondary_tables = classes[k].secondary;
            g->v[k].vt.slots = classes[k].slots;
            g->v[k].vt.nslots = classes[k].nslots;
        }
        free(classes);

        fprintf(stderr, "info: found %zu class vtables in %s\n", nclasses, VT_MODULES[i]);
    }

    return map;
}
