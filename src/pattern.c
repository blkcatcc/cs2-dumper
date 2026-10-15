#include "dumper.h"

static int hex2(const char *s) {
    int v = 0;
    for (int i = 0; i < 2; i++) {
        char c = s[i];
        int d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        else return -1;
        v = v * 16 + d;
    }
    return v;
}

int pattern_parse(Pattern *pat, const char *text) {
    size_t cap = 64, n = 0;
    u8 *bytes = xmalloc(cap), *mask = xmalloc(cap);

    const char *p = text;
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\n') p++;
        if (!*p) break;

        size_t start = 0;
        while (p[start] && p[start] != ' ' && p[start] != '\t' && p[start] != '\n') start++;

        if (start == 1 && p[0] == '?') {
            if (n == cap) {
                cap *= 2;
                bytes = xrealloc(bytes, cap);
                mask = xrealloc(mask, cap);
            }
            bytes[n] = 0;
            mask[n] = 0;
            n++;
        } else if (start == 2 && p[0] == '?' && p[1] == '?') {
            if (n == cap) {
                cap *= 2;
                bytes = xrealloc(bytes, cap);
                mask = xrealloc(mask, cap);
            }
            bytes[n] = 0;
            mask[n] = 0;
            n++;
        } else if (start == 2) {
            int v = hex2(p);
            if (v < 0) {
                free(bytes);
                free(mask);
                return 0;
            }
            if (n == cap) {
                cap *= 2;
                bytes = xrealloc(bytes, cap);
                mask = xrealloc(mask, cap);
            }
            bytes[n] = (u8)v;
            mask[n] = 1;
            n++;
        } else {
            free(bytes);
            free(mask);
            return 0;
        }
        p += start;
    }

    if (n == 0) {
        free(bytes);
        free(mask);
        return 0;
    }
    pat->bytes = bytes;
    pat->mask = mask;
    pat->n = n;
    return 1;
}

void pattern_free(Pattern *pat) {
    free(pat->bytes);
    free(pat->mask);
    pat->bytes = NULL;
    pat->mask = NULL;
    pat->n = 0;
}

static int match_at(const Pattern *pat, const u8 *buf, size_t len, size_t at) {
    if (at + pat->n > len) return 0;
    for (size_t i = 0; i < pat->n; i++) {
        if (pat->mask[i] && buf[at + i] != pat->bytes[i]) return 0;
    }
    return 1;
}

int pattern_scan(const Pattern *pat, const u8 *buf, size_t len, size_t *off) {
    if (pat->n == 0 || pat->n > len) return 0;
    size_t last = len - pat->n;

    /* When the first byte is fixed, memchr skips most of the buffer. */
    if (pat->mask[0]) {
        const u8 *p = buf;
        size_t remaining = last + 1;
        while (remaining) {
            const u8 *hit = memchr(p, pat->bytes[0], remaining);
            if (!hit) return 0;
            size_t at = (size_t)(hit - buf);
            if (match_at(pat, buf, len, at)) {
                *off = at;
                return 1;
            }
            p = hit + 1;
            remaining = last + 1 - (size_t)(p - buf);
        }
        return 0;
    }

    for (size_t at = 0; at <= last; at++)
        if (match_at(pat, buf, len, at)) {
            *off = at;
            return 1;
        }
    return 0;
}

u64 resolve_rip(const Mem *mem, u64 addr, u64 offset, u64 len) {
    u32 raw = mem_read_u32(mem, addr + offset);
    i32 disp = (i32)raw;
    return (u64)((i64)addr + (i64)disp + (i64)len);
}
