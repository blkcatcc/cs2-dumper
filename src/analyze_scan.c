#include "dumper.h"

/* Reads a module into a contiguous buffer indexed from its load address.
   Falls back to a page walk when the mapping has unreadable gaps. */
u8 *read_module(const Mem *mem, const Module *m, size_t *out_len) {
    size_t len = (size_t)m->size;
    if (len == 0) return NULL;

    u8 *buf = xmalloc(len);
    if (mem_read(mem, m->base, buf, len)) {
        *out_len = len;
        return buf;
    }

    const size_t PAGE = 0x1000;
    memset(buf, 0, len);
    size_t ok = 0;
    for (size_t off = 0; off < len; off += PAGE) {
        size_t n = PAGE < len - off ? PAGE : len - off;
        if (mem_read(mem, m->base + off, buf + off, n)) ok++;
    }
    if (ok == 0) {
        free(buf);
        return NULL;
    }
    *out_len = len;
    return buf;
}

static int cmp_nameval(const void *a, const void *b) {
    return strcmp(((const NameVal *)a)->name, ((const NameVal *)b)->name);
}

void nv_push(NVMap *map, const char *module, const NameVal *vals, size_t n) {
    if (n == 0) return;
    NVGroup *g = &map->v[map->n++];
    g->module = xstrdup(module);
    g->v = xmalloc(n * sizeof(*g->v));
    memcpy(g->v, vals, n * sizeof(*g->v));
    qsort(g->v, n, sizeof(*g->v), cmp_nameval);
    g->n = n;
}

/* ------------------------------------------------------------ offsets */

static u64 apply_ops(const Mem *mem, u64 addr, const Sig *sig) {
    u64 cur = addr;
    for (size_t i = 0; i < sig->nops; i++) {
        const Op *op = &sig->ops[i];
        switch (op->kind) {
        case OP_ADD:
            cur += op->value;
            break;
        case OP_SUB:
            cur -= op->value;
            break;
        case OP_RIP:
            cur = resolve_rip(mem, cur, op->offset, op->len);
            break;
        case OP_READ:
            cur = mem_read_u64(mem, cur);
            break;
        case OP_SLICE: {
            size_t n = (size_t)(op->end - op->start);
            u8 bytes[8] = { 0 };
            if (n > 8) n = 8;
            mem_read(mem, cur + op->start, bytes, n);
            memcpy(&cur, bytes, 8);
            break;
        }
        }
    }
    return cur;
}

NVMap analyze_offsets(const Mem *mem, const Modules *mods, const Config *cfg) {
    NVMap map;
    map.v = xcalloc(cfg->ngroups ? cfg->ngroups : 1, sizeof(*map.v));
    map.n = 0;

    for (size_t g = 0; g < cfg->ngroups; g++) {
        const SigGroup *sg = &cfg->groups[g];
        const Module *mod = modules_get(mods, sg->module);
        if (!mod) {
            fprintf(stderr, "warning: skipping signatures for %s: module not found\n",
                    sg->module);
            continue;
        }

        size_t len = 0;
        u8 *buf = read_module(mem, mod, &len);
        if (!buf) {
            fprintf(stderr, "warning: skipping signatures for %s: unable to read module\n",
                    sg->module);
            continue;
        }

        NameVal *vals = xcalloc(sg->nsigs ? sg->nsigs : 1, sizeof(*vals));
        size_t n = 0;

        for (size_t s = 0; s < sg->nsigs; s++) {
            const Sig *sig = &sg->sigs[s];
            Pattern pat;
            if (!pattern_parse(&pat, sig->pattern)) {
                fprintf(stderr, "error: unable to parse signature: %s\n", sig->name);
                continue;
            }
            size_t off = 0;
            if (!pattern_scan(&pat, buf, len, &off)) {
                fprintf(stderr, "error: unable to find signature for: %s\n", sig->name);
                pattern_free(&pat);
                continue;
            }
            pattern_free(&pat);

            u64 result = apply_ops(mem, mod->base + off, sig);

            /* Signatures that end in `slice` yield a plain integer, not an
               address; subtracting the base would underflow. Prefer the
               module-relative value and fall back to the low 32 bits. */
            u64 rva = result - mod->base;
            vals[n].name = xstrdup(sig->name);
            vals[n].value = rva <= 0xFFFFFFFFULL ? rva : (u64)(u32)result;
            n++;
        }

        free(buf);
        nv_push(&map, sg->module, vals, n);
        free(vals);
    }

    return map;
}

u32 analyze_build_number(const Mem *mem, const Modules *mods, const NVMap *offsets) {
    for (size_t g = 0; g < offsets->n; g++) {
        const NVGroup *grp = &offsets->v[g];
        for (size_t i = 0; i < grp->n; i++) {
            if (strcmp(grp->v[i].name, "dwBuildNumber") != 0) continue;
            const Module *mod = modules_get(mods, grp->module);
            if (!mod) continue;
            u32 build = mem_read_u32(mem, mod->base + grp->v[i].value);
            if (build) return build;
        }
    }
    return 0;
}

/* ------------------------------------------------------------ buttons */

#define KEYBUTTON_STATE 0x30
#define KEYBUTTON_NEXT 0x88

NVMap analyze_buttons(const Mem *mem, const Modules *mods) {
    NVMap map;
    map.v = xcalloc(1, sizeof(*map.v));
    map.n = 0;

    const Module *mod = modules_get(mods, "libclient.so");
    if (!mod) {
        fprintf(stderr, "warning: skipping buttons: libclient.so not found\n");
        return map;
    }

    size_t len = 0;
    u8 *buf = read_module(mem, mod, &len);
    if (!buf) {
        fprintf(stderr, "warning: skipping buttons: unable to read libclient.so\n");
        return map;
    }

    /* Anchored on the KeyButton registrar: head = this; this->next = old_head. */
    Pattern pat;
    size_t off = 0;
    int found = pattern_parse(&pat, "48 8B 05 ? ? ? ? 48 89 1D ? ? ? ? 48 89 83 88 00 00 00") &&
                pattern_scan(&pat, buf, len, &off);
    pattern_free(&pat);
    free(buf);

    if (!found) {
        fprintf(stderr, "error: unable to find the button list signature\n");
        return map;
    }

    u64 list_addr = resolve_rip(mem, mod->base + off, 3, 7);
    u64 node = mem_read_u64(mem, list_addr);

    size_t cap = 64, n = 0;
    NameVal *vals = xcalloc(cap, sizeof(*vals));

    while (node) {
        char *name = mem_read_str(mem, mem_read_u64(mem, node + 8), 32);
        if (n == cap) {
            cap *= 2;
            vals = xrealloc(vals, cap * sizeof(*vals));
        }
        vals[n].name = name;
        vals[n].value = (node + KEYBUTTON_STATE) - mod->base;
        n++;
        node = mem_read_u64(mem, node + KEYBUTTON_NEXT);
    }

    nv_push(&map, "libclient.so", vals, n);
    free(vals);
    return map;
}

/* --------------------------------------------------------- interfaces */

#define INTERFACE_CREATE_FN 0
#define INTERFACE_NAME 8
#define INTERFACE_NEXT 0x10

/* The list head is loaded in the prologue of the exported CreateInterface
   function as `mov rbx, [rip + disp32]`; scanning only that prologue keeps the
   match unambiguous. */
static int interface_list_head(const Mem *mem, const Module *mod, const u8 *buf, size_t len,
                               u64 *head) {
    Image view;
    memset(&view, 0, sizeof(view));
    view.data = (u8 *)buf;
    view.len = len;
    view.base = mod->base;

    u64 export = image_export(&view, "CreateInterface");
    if (!export) return 0;

    size_t start = (size_t)export;
    size_t end = start + 0x40;
    if (end > len) end = len;
    if (start >= end) return 0;

    Pattern pat;
    if (!pattern_parse(&pat, "48 8B 1D ? ? ? ? 48 85 DB")) return 0;
    size_t off = 0;
    int found = pattern_scan(&pat, buf + start, end - start, &off);
    pattern_free(&pat);
    if (!found) return 0;

    u64 list_ptr = resolve_rip(mem, mod->base + start + off, 3, 7);
    *head = mem_read_u64(mem, list_ptr);
    return 1;
}

static NameVal *read_interfaces(const Mem *mem, const Module *mod, u64 head, size_t *count) {
    size_t cap = 64, n = 0;
    NameVal *vals = xcalloc(cap, sizeof(*vals));

    u64 cur = head;
    size_t guard = 0;
    while (cur && guard++ < 100000) {
        u64 create_fn = mem_read_u64(mem, cur + INTERFACE_CREATE_FN);
        char *name = mem_read_str(mem, mem_read_u64(mem, cur + INTERFACE_NAME), 128);

        if (n == cap) {
            cap *= 2;
            vals = xrealloc(vals, cap * sizeof(*vals));
        }
        vals[n].name = name;
        vals[n].value = create_fn >= mod->base ? create_fn - mod->base : create_fn;
        n++;

        cur = mem_read_u64(mem, cur + INTERFACE_NEXT);
    }

    *count = n;
    return vals;
}

NVMap analyze_interfaces(const Mem *mem, const Modules *mods) {
    NVMap map;
    map.v = xcalloc(mods->n ? mods->n : 1, sizeof(*map.v));
    map.n = 0;

    for (size_t i = 0; i < mods->n; i++) {
        const Module *mod = &mods->v[i];

        size_t len = 0;
        u8 *buf = read_module(mem, mod, &len);
        if (!buf) continue;

        u64 head = 0;
        int have = interface_list_head(mem, mod, buf, len, &head);
        free(buf);
        if (!have) continue;

        size_t n = 0;
        NameVal *vals = read_interfaces(mem, mod, head, &n);
        if (n == 0) {
            free(vals);
            continue;
        }
        nv_push(&map, mod->name, vals, n);
        free(vals);
    }

    return map;
}

