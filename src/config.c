#include "dumper.h"

#include <errno.h>
#include <unistd.h>

static char *read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);
    char *buf = xmalloc((size_t)sz + 1);
    size_t got = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[got] = 0;
    if (len) *len = got;
    return buf;
}

/* Working directory first, then next to the executable. */
static char *config_path(void) {
    if (access("config.json", R_OK) == 0) return xstrdup("config.json");

    char exe[4096];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n <= 0) return NULL;
    exe[n] = 0;
    char *slash = strrchr(exe, '/');
    if (!slash) return NULL;
    *slash = 0;

    size_t need = strlen(exe) + strlen("/config.json") + 1;
    char *path = xmalloc(need);
    snprintf(path, need, "%s/config.json", exe);
    if (access(path, R_OK) != 0) {
        free(path);
        return NULL;
    }
    return path;
}

static void parse_sig(Json *j, Sig *sig) {
    sig->name = xstrdup(json_str(j, "name", ""));
    sig->pattern = xstrdup(json_str(j, "pattern", ""));

    Json *ops = json_get(j, "operations");
    if (!ops || ops->kind != J_ARR) return;

    sig->ops = xcalloc(ops->n ? ops->n : 1, sizeof(*sig->ops));
    for (size_t i = 0; i < ops->n; i++) {
        Json *o = ops->items[i];
        const char *type = json_str(o, "type", "");
        Op *op = &sig->ops[sig->nops];
        if (strcmp(type, "add") == 0) {
            op->kind = OP_ADD;
            op->value = (u64)json_num(o, "value", 0);
        } else if (strcmp(type, "sub") == 0) {
            op->kind = OP_SUB;
            op->value = (u64)json_num(o, "value", 0);
        } else if (strcmp(type, "rip") == 0) {
            op->kind = OP_RIP;
            op->offset = (u64)json_num(o, "offset", 3);
            op->len = (u64)json_num(o, "length", json_num(o, "len", 7));
        } else if (strcmp(type, "read") == 0) {
            op->kind = OP_READ;
        } else if (strcmp(type, "slice") == 0) {
            op->kind = OP_SLICE;
            op->start = (u64)json_num(o, "start", 0);
            op->end = (u64)json_num(o, "end", 0);
        } else {
            continue;
        }
        sig->nops++;
    }
}

Config *config_load(void) {
    char *path = config_path();
    if (!path) {
        fprintf(stderr, "error: config.json not found\n"
                        "Looked in the current directory and next to the executable.\n");
        exit(1);
    }

    size_t len = 0;
    char *text = read_file(path, &len);
    if (!text) {
        fprintf(stderr, "error: unable to read %s: %s\n", path, strerror(errno));
        exit(1);
    }

    Json *root = json_parse(text, len);
    if (!root) {
        fprintf(stderr, "error: unable to parse %s\n", path);
        exit(1);
    }

    Config *cfg = xcalloc(1, sizeof(*cfg));
    cfg->executable = xstrdup(json_str(root, "executable", "cs2"));

    Json *sigs = json_get(root, "signatures");
    if (sigs && sigs->kind == J_ARR) {
        /* One SigGroup per module key; a group object holds several modules. */
        size_t groups_cap = 8;
        cfg->groups = xcalloc(groups_cap, sizeof(*cfg->groups));
        for (size_t g = 0; g < sigs->n; g++) {
            Json *grp = sigs->items[g];
            if (grp->kind != J_OBJ) continue;
            for (size_t m = 0; m < grp->n; m++) {
                Json *list = grp->items[m];
                if (list->kind != J_ARR) continue;
                if (cfg->ngroups == groups_cap) {
                    groups_cap *= 2;
                    cfg->groups = xrealloc(cfg->groups, groups_cap * sizeof(*cfg->groups));
                    memset(cfg->groups + cfg->ngroups, 0,
                           (groups_cap - cfg->ngroups) * sizeof(*cfg->groups));
                }
                SigGroup *sg = &cfg->groups[cfg->ngroups++];
                sg->module = xstrdup(grp->keys[m]);
                sg->sigs = xcalloc(list->n ? list->n : 1, sizeof(*sg->sigs));
                for (size_t s = 0; s < list->n; s++) {
                    parse_sig(list->items[s], &sg->sigs[sg->nsigs]);
                    sg->nsigs++;
                }
            }
        }
    }

    json_free(root);
    free(text);
    free(path);
    return cfg;
}
