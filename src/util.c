#include "dumper.h"

#include <ctype.h>
#include <stdarg.h>

void *xmalloc(size_t n) {
    void *p = malloc(n ? n : 1);
    if (!p) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    return p;
}

void *xcalloc(size_t n, size_t sz) {
    void *p = calloc(n ? n : 1, sz ? sz : 1);
    if (!p) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    return p;
}

void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n ? n : 1);
    if (!q) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }
    return q;
}

char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = xmalloc(n);
    memcpy(p, s, n);
    return p;
}

void buf_init(Buf *b) {
    b->cap = 256;
    b->len = 0;
    b->p = xmalloc(b->cap);
    b->p[0] = 0;
}

void buf_add(Buf *b, const char *s, size_t n) {
    if (b->len + n + 1 > b->cap) {
        while (b->len + n + 1 > b->cap) b->cap *= 2;
        b->p = xrealloc(b->p, b->cap);
    }
    memcpy(b->p + b->len, s, n);
    b->len += n;
    b->p[b->len] = 0;
}

void buf_puts(Buf *b, const char *s) {
    buf_add(b, s, strlen(s));
}

void buf_printf(Buf *b, const char *fmt, ...) {
    va_list ap;
    char tmp[1024];
    va_start(ap, fmt);
    int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if ((size_t)n < sizeof(tmp)) {
        buf_add(b, tmp, (size_t)n);
        return;
    }
    char *big = xmalloc((size_t)n + 1);
    va_start(ap, fmt);
    vsnprintf(big, (size_t)n + 1, fmt, ap);
    va_end(ap);
    buf_add(b, big, (size_t)n);
    free(big);
}

void strs_push(Strs *s, const char *str) {
    if (s->n == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 16;
        s->v = xrealloc(s->v, s->cap * sizeof(*s->v));
    }
    s->v[s->n++] = xstrdup(str);
}

int strs_find(const Strs *s, const char *str) {
    for (size_t i = 0; i < s->n; i++)
        if (strcmp(s->v[i], str) == 0) return (int)i;
    return -1;
}

void strs_free(Strs *s) {
    for (size_t i = 0; i < s->n; i++) free(s->v[i]);
    free(s->v);
    s->v = NULL;
    s->n = s->cap = 0;
}

char *str_lower(const char *s) {
    size_t n = strlen(s);
    char *o = xmalloc(n + 1);
    for (size_t i = 0; i < n; i++) o[i] = (char)tolower((unsigned char)s[i]);
    o[n] = 0;
    return o;
}

int str_starts(const char *a, const char *prefix) {
    return strncmp(a, prefix, strlen(prefix)) == 0;
}

/* Upstream MutCString::prepend: keep the existing text, put `dst` in front,
   skipping the insert when `dst` is already present. */
char *str_prepend_once(const char *dst, const char *src) {
    size_t dn = strlen(dst), sn = strlen(src);
    char *o = xmalloc(dn + sn + 1);
    if (strstr(src, dst)) {
        memcpy(o, src, sn + 1);
    } else {
        memcpy(o, dst, dn);
        memcpy(o + dn, src, sn + 1);
    }
    return o;
}

/* --------------------------------------------------------------- naming */

/* Rust keywords that cannot appear as a bare identifier. `Self`/`self` and
   `crate`/`super` cannot be raw-escaped at all, so those get a suffix. */
static const char *rust_keywords[] = {
    "as","async","await","break","const","continue","crate","dyn","else","enum","extern",
    "false","fn","for","if","impl","in","let","loop","match","mod","move","mut","pub",
    "ref","return","self","Self","static","struct","super","trait","true","type","union",
    "unsafe","use","where","while","abstract","become","box","do","final","macro",
    "override","priv","typeof","unsized","virtual","yield","try",
};

static int rust_keyword(const char *s) {
    for (size_t i = 0; i < sizeof(rust_keywords) / sizeof(rust_keywords[0]); i++)
        if (strcmp(rust_keywords[i], s) == 0) return 1;
    return 0;
}

/* Returns a Rust-safe identifier; caller frees when it differs from `s`. */
char *name_rust(const char *s) {
    if (!rust_keyword(s)) return xstrdup(s);
    /* `Self`, `self`, `super`, `crate` are reserved in a way that `r#` cannot
       rescue, so suffix those instead. */
    if (strcmp(s, "Self") == 0 || strcmp(s, "self") == 0 || strcmp(s, "super") == 0 ||
        strcmp(s, "crate") == 0) {
        size_t n = strlen(s) + 2;
        char *o = xmalloc(n);
        snprintf(o, n, "%s_", s);
        return o;
    }
    size_t n = strlen(s) + 3;
    char *o = xmalloc(n);
    snprintf(o, n, "r#%s", s);
    return o;
}

#define MAX_WORDS 64

/* Splits on non-alphanumerics, digit boundaries and lower-to-upper case
   changes, then re-joins. Mirrors the reference dump's case conversion, which
   keeps a digit-led run untouched (`libclient_so` -> `LibclientSo`). */
static size_t split_words(const char *s, char *words[MAX_WORDS]) {
    size_t n = 0;
    Buf cur;
    buf_init(&cur);

    for (const char *p = s;; p++) {
        char c = *p;
        if (!c || !isalnum((unsigned char)c)) {
            if (cur.len && n < MAX_WORDS) words[n++] = xstrdup(cur.p);
            cur.len = 0;
            if (cur.p) cur.p[0] = 0;
            if (!c) break;
            continue;
        }
        if (cur.len) {
            char prev = cur.p[cur.len - 1];
            int boundary = (isdigit((unsigned char)c) != isdigit((unsigned char)prev)) ||
                           (isupper((unsigned char)c) && !isupper((unsigned char)prev));
            if (boundary) {
                if (n < MAX_WORDS) words[n++] = xstrdup(cur.p);
                cur.len = 0;
                cur.p[0] = 0;
            }
        }
        buf_add(&cur, &c, 1);
    }
    free(cur.p);
    return n;
}

static void free_words(char *words[MAX_WORDS], size_t n) {
    for (size_t i = 0; i < n; i++) free(words[i]);
}

char *name_slugify(const char *s) {
    size_t n = strlen(s);
    char *o = xmalloc(n + 1);
    for (size_t i = 0; i < n; i++)
        o[i] = isalnum((unsigned char)s[i]) ? s[i] : '_';
    o[n] = 0;
    return o;
}

char *name_snake(const char *s) {
    char *words[MAX_WORDS];
    size_t n = split_words(s, words);
    Buf b;
    buf_init(&b);
    for (size_t i = 0; i < n; i++) {
        if (i) buf_puts(&b, "_");
        if (isdigit((unsigned char)words[i][0])) {
            buf_puts(&b, words[i]);
        } else {
            for (char *p = words[i]; *p; p++) {
                char c = (char)tolower((unsigned char)*p);
                buf_add(&b, &c, 1);
            }
        }
    }
    free_words(words, n);
    return b.p;
}

char *name_pascal(const char *s) {
    char *words[MAX_WORDS];
    size_t n = split_words(s, words);
    Buf b;
    buf_init(&b);
    for (size_t i = 0; i < n; i++) {
        if (isdigit((unsigned char)words[i][0])) {
            buf_puts(&b, words[i]);
            continue;
        }
        char c = (char)toupper((unsigned char)words[i][0]);
        buf_add(&b, &c, 1);
        for (char *p = words[i] + 1; *p; p++) {
            char l = (char)tolower((unsigned char)*p);
            buf_add(&b, &l, 1);
        }
    }
    free_words(words, n);
    return b.p;
}

static int zig_keyword(const char *s) {
    static const char *kw[] = {
        "addrspace","align","allowzero","and","anyframe","anytype","asm","async","await",
        "break","callconv","catch","comptime","const","continue","defer","else","enum",
        "errdefer","error","export","extern","false","fn","for","if","inline","linksection",
        "noalias","noinline","nosuspend","null","opaque","or","orelse","packed","pub",
        "resume","return","struct","suspend","switch","test","threadlocal","true","try",
        "union","unreachable","usingnamespace","var","volatile","while",
    };
    for (size_t i = 0; i < sizeof(kw) / sizeof(kw[0]); i++)
        if (strcmp(kw[i], s) == 0) return 1;
    return 0;
}

/* Quoted with @"..." unless the name is a plain identifier. */
char *name_zig(const char *s) {
    int ident = s[0] == '_' || isalpha((unsigned char)s[0]);
    for (const char *p = s; *p && ident; p++)
        if (*p != '_' && !isalnum((unsigned char)*p)) ident = 0;

    if (ident && !zig_keyword(s)) return xstrdup(s);

    Buf b;
    buf_init(&b);
    buf_puts(&b, "@\"");
    for (const char *p = s; *p; p++) {
        if (*p == '\\' || *p == '"') buf_puts(&b, "\\");
        buf_add(&b, p, 1);
    }
    buf_puts(&b, "\"");
    return b.p;
}

/* Module-name helpers: slugify, then convert, freeing the intermediate. */
char *name_snake_of(const char *s) {
    char *slug = name_slugify(s);
    char *out = name_snake(slug);
    free(slug);
    return out;
}

char *name_pascal_of(const char *s) {
    char *slug = name_slugify(s);
    char *out = name_pascal(slug);
    free(slug);
    return out;
}

char *name_zig_of(const char *s) {
    char *snake = name_snake_of(s);
    char *out = name_zig(snake);
    free(snake);
    return out;
}

/* --------------------------------------------------------------- output */

#include <sys/stat.h>

void write_file(const char *dir, const char *base, const char *ext, const Buf *b) {
    size_t need = strlen(dir) + strlen(base) + strlen(ext) + 3;
    char *path = xmalloc(need);
    snprintf(path, need, "%s/%s.%s", dir, base, ext);
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "error: unable to write %s\n", path);
        free(path);
        return;
    }
    fwrite(b->p, 1, b->len, f);
    fclose(f);
    free(path);
}
