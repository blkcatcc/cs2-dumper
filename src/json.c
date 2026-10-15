#include "dumper.h"

#include <ctype.h>

typedef struct {
    const char *s;
    size_t n, i;
    int err;
} P;

static void skip_ws(P *p) {
    while (p->i < p->n) {
        char c = p->s[p->i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') p->i++;
        else break;
    }
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void utf8_append(Buf *b, unsigned cp) {
    if (cp < 0x80) {
        char c = (char)cp;
        buf_add(b, &c, 1);
    } else if (cp < 0x800) {
        char o[2] = { (char)(0xC0 | (cp >> 6)), (char)(0x80 | (cp & 0x3F)) };
        buf_add(b, o, 2);
    } else if (cp < 0x10000) {
        char o[3] = { (char)(0xE0 | (cp >> 12)), (char)(0x80 | ((cp >> 6) & 0x3F)),
                      (char)(0x80 | (cp & 0x3F)) };
        buf_add(b, o, 3);
    } else {
        char o[4] = { (char)(0xF0 | (cp >> 18)), (char)(0x80 | ((cp >> 12) & 0x3F)),
                      (char)(0x80 | ((cp >> 6) & 0x3F)), (char)(0x80 | (cp & 0x3F)) };
        buf_add(b, o, 4);
    }
}

static char *parse_string(P *p) {
    if (p->i >= p->n || p->s[p->i] != '"') {
        p->err = 1;
        return NULL;
    }
    p->i++;
    Buf b;
    buf_init(&b);
    while (p->i < p->n) {
        unsigned char c = (unsigned char)p->s[p->i++];
        if (c == '"') return b.p;
        if (c != '\\') {
            buf_add(&b, (char *)&c, 1);
            continue;
        }
        if (p->i >= p->n) break;
        char e = p->s[p->i++];
        switch (e) {
        case '"': buf_puts(&b, "\""); break;
        case '\\': buf_puts(&b, "\\"); break;
        case '/': buf_puts(&b, "/"); break;
        case 'b': buf_puts(&b, "\b"); break;
        case 'f': buf_puts(&b, "\f"); break;
        case 'n': buf_puts(&b, "\n"); break;
        case 'r': buf_puts(&b, "\r"); break;
        case 't': buf_puts(&b, "\t"); break;
        case 'u': {
            unsigned cp = 0;
            for (int k = 0; k < 4; k++) {
                if (p->i >= p->n) { p->err = 1; break; }
                int h = hexval(p->s[p->i++]);
                if (h < 0) { p->err = 1; break; }
                cp = cp * 16 + (unsigned)h;
            }
            if (cp >= 0xD800 && cp <= 0xDBFF && p->i + 1 < p->n && p->s[p->i] == '\\' &&
                p->s[p->i + 1] == 'u') {
                p->i += 2;
                unsigned lo = 0;
                for (int k = 0; k < 4; k++) {
                    if (p->i >= p->n) { p->err = 1; break; }
                    int h = hexval(p->s[p->i++]);
                    if (h < 0) { p->err = 1; break; }
                    lo = lo * 16 + (unsigned)h;
                }
                if (lo >= 0xDC00 && lo <= 0xDFFF)
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
            }
            utf8_append(&b, cp);
            break;
        }
        default:
            p->err = 1;
            break;
        }
        if (p->err) {
            free(b.p);
            return NULL;
        }
    }
    free(b.p);
    p->err = 1;
    return NULL;
}

static Json *new_json(JKind k) {
    Json *j = xcalloc(1, sizeof(*j));
    j->kind = k;
    return j;
}

static void push_item(Json *j, Json *item, char *key) {
    j->items = xrealloc(j->items, (j->n + 1) * sizeof(*j->items));
    j->items[j->n] = item;
    if (j->kind == J_OBJ) {
        j->keys = xrealloc(j->keys, (j->n + 1) * sizeof(*j->keys));
        j->keys[j->n] = key;
    }
    j->n++;
}

static Json *parse_value(P *p);

static Json *parse_object(P *p) {
    Json *j = new_json(J_OBJ);
    p->i++;
    skip_ws(p);
    if (p->i < p->n && p->s[p->i] == '}') {
        p->i++;
        return j;
    }
    for (;;) {
        skip_ws(p);
        char *key = parse_string(p);
        if (!key) {
            json_free(j);
            return NULL;
        }
        skip_ws(p);
        if (p->i >= p->n || p->s[p->i] != ':') {
            free(key);
            json_free(j);
            p->err = 1;
            return NULL;
        }
        p->i++;
        Json *v = parse_value(p);
        if (!v) {
            free(key);
            json_free(j);
            return NULL;
        }
        push_item(j, v, key);
        skip_ws(p);
        if (p->i < p->n && p->s[p->i] == ',') {
            p->i++;
            continue;
        }
        if (p->i < p->n && p->s[p->i] == '}') {
            p->i++;
            return j;
        }
        json_free(j);
        p->err = 1;
        return NULL;
    }
}

static Json *parse_array(P *p) {
    Json *j = new_json(J_ARR);
    p->i++;
    skip_ws(p);
    if (p->i < p->n && p->s[p->i] == ']') {
        p->i++;
        return j;
    }
    for (;;) {
        Json *v = parse_value(p);
        if (!v) {
            json_free(j);
            return NULL;
        }
        push_item(j, v, NULL);
        skip_ws(p);
        if (p->i < p->n && p->s[p->i] == ',') {
            p->i++;
            continue;
        }
        if (p->i < p->n && p->s[p->i] == ']') {
            p->i++;
            return j;
        }
        json_free(j);
        p->err = 1;
        return NULL;
    }
}

static Json *parse_number(P *p) {
    size_t start = p->i;
    if (p->i < p->n && (p->s[p->i] == '-' || p->s[p->i] == '+')) p->i++;
    while (p->i < p->n &&
           (isdigit((unsigned char)p->s[p->i]) || p->s[p->i] == '.' || p->s[p->i] == 'e' ||
            p->s[p->i] == 'E' || p->s[p->i] == '+' || p->s[p->i] == '-'))
        p->i++;
    if (p->i == start) {
        p->err = 1;
        return NULL;
    }
    char tmp[64];
    size_t len = p->i - start;
    if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
    memcpy(tmp, p->s + start, len);
    tmp[len] = 0;
    Json *j = new_json(J_NUM);
    j->num = strtod(tmp, NULL);
    return j;
}

static Json *parse_value(P *p) {
    skip_ws(p);
    if (p->i >= p->n) {
        p->err = 1;
        return NULL;
    }
    char c = p->s[p->i];
    if (c == '{') return parse_object(p);
    if (c == '[') return parse_array(p);
    if (c == '"') {
        char *s = parse_string(p);
        if (!s) return NULL;
        Json *j = new_json(J_STR);
        j->str = s;
        return j;
    }
    if (strncmp(p->s + p->i, "true", 4) == 0) {
        p->i += 4;
        Json *j = new_json(J_BOOL);
        j->boolean = 1;
        return j;
    }
    if (strncmp(p->s + p->i, "false", 5) == 0) {
        p->i += 5;
        Json *j = new_json(J_BOOL);
        j->boolean = 0;
        return j;
    }
    if (strncmp(p->s + p->i, "null", 4) == 0) {
        p->i += 4;
        return new_json(J_NULL);
    }
    if (c == '-' || c == '+' || isdigit((unsigned char)c)) return parse_number(p);
    p->err = 1;
    return NULL;
}

Json *json_parse(const char *text, size_t len) {
    P p = { text, len, 0, 0 };
    Json *j = parse_value(&p);
    if (!j || p.err) {
        json_free(j);
        return NULL;
    }
    return j;
}

void json_free(Json *j) {
    if (!j) return;
    for (size_t i = 0; i < j->n; i++) {
        json_free(j->items[i]);
        if (j->kind == J_OBJ && j->keys) free(j->keys[i]);
    }
    free(j->items);
    free(j->keys);
    free(j->str);
    free(j);
}

Json *json_get(const Json *o, const char *key) {
    if (!o || o->kind != J_OBJ) return NULL;
    for (size_t i = 0; i < o->n; i++)
        if (strcmp(o->keys[i], key) == 0) return o->items[i];
    return NULL;
}

const char *json_str(const Json *j, const char *key, const char *def) {
    Json *v = json_get(j, key);
    if (!v || v->kind != J_STR) return def;
    return v->str;
}

double json_num(const Json *j, const char *key, double def) {
    Json *v = json_get(j, key);
    if (!v || v->kind != J_NUM) return def;
    return v->num;
}

/* ------------------------------------------------------------- writing */

void json_escape(Buf *b, const char *s) {
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        switch (*p) {
        case '"': buf_puts(b, "\\\""); break;
        case '\\': buf_puts(b, "\\\\"); break;
        case '\n': buf_puts(b, "\\n"); break;
        case '\r': buf_puts(b, "\\r"); break;
        case '\t': buf_puts(b, "\\t"); break;
        case '\b': buf_puts(b, "\\b"); break;
        case '\f': buf_puts(b, "\\f"); break;
        default:
            if (*p < 0x20) buf_printf(b, "\\u%04x", *p);
            else buf_add(b, (const char *)p, 1);
            break;
        }
    }
}

void json_indent(Buf *b, int level, int size) {
    for (int i = 0; i < level * size; i++) buf_puts(b, " ");
}
