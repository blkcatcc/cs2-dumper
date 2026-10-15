#include "dumper.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/* ------------------------------------------------------------------ mem */

int mem_open(Mem *m, pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/mem", (int)pid);
    m->fd = open(path, O_RDONLY);
    m->pid = pid;
    return m->fd >= 0;
}

void mem_close(Mem *m) {
    if (m->fd >= 0) close(m->fd);
    m->fd = -1;
}

int mem_read(const Mem *m, u64 addr, void *dst, size_t n) {
    u8 *p = dst;
    size_t done = 0;
    while (done < n) {
        ssize_t got = pread(m->fd, p + done, n - done, (off_t)(addr + done));
        if (got <= 0) return 0;
        done += (size_t)got;
    }
    return 1;
}

u64 mem_read_u64(const Mem *m, u64 addr) {
    u64 v = 0;
    mem_read(m, addr, &v, 8);
    return v;
}

u32 mem_read_u32(const Mem *m, u64 addr) {
    u32 v = 0;
    mem_read(m, addr, &v, 4);
    return v;
}

char *mem_read_str(const Mem *m, u64 addr, size_t max) {
    char *buf = xcalloc(max + 1, 1);
    if (!mem_read(m, addr, buf, max)) {
        buf[0] = 0;
        return buf;
    }
    buf[max] = 0;
    return buf;
}

/* --------------------------------------------------------------- modules */

pid_t proc_find(const char *name) {
    DIR *d = opendir("/proc");
    if (!d) return -1;

    pid_t best = -1;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (!isdigit((unsigned char)e->d_name[0])) continue;
        char path[300];
        snprintf(path, sizeof(path), "/proc/%s/comm", e->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        char comm[256] = { 0 };
        if (!fgets(comm, sizeof(comm), f)) {
            fclose(f);
            continue;
        }
        fclose(f);
        size_t n = strlen(comm);
        while (n && (comm[n - 1] == '\n' || comm[n - 1] == '\r')) comm[--n] = 0;
        if (strcmp(comm, name) != 0) continue;

        /* Prefer the oldest matching process. */
        pid_t pid = (pid_t)atoi(e->d_name);
        if (best < 0 || pid < best) best = pid;
    }
    closedir(d);
    return best;
}

/* Shortens a path to its basename and strips a " (deleted)" suffix. */
static char *module_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    const char *name = slash ? slash + 1 : path;
    char *out = xstrdup(name);
    char *suf = strstr(out, " (deleted)");
    if (suf) *suf = 0;
    return out;
}

int modules_enum(pid_t pid, Modules *out) {
    memset(out, 0, sizeof(*out));

    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", (int)pid);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    size_t cap = 128;
    out->v = xcalloc(cap, sizeof(*out->v));

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        unsigned long start = 0, end = 0;
        char perms[8] = { 0 };
        if (sscanf(line, "%lx-%lx %7s", &start, &end, perms) != 3) continue;

        char *p = strchr(line, '/');
        if (!p) continue; /* anonymous mapping */
        char *nl = strchr(p, '\n');
        if (nl) *nl = 0;

        char *name = module_basename(p);
        if (!name[0] || !strstr(name, ".so")) {
            free(name);
            continue;
        }

        const Module *ex = modules_get(out, name);
        if (ex) {
            if ((u64)start < ex->base) ((Module *)ex)->base = start;
            if ((u64)end > ex->base + ex->size) ((Module *)ex)->size = end - ex->base;
            free(name);
            continue;
        }

        if (out->n == cap) {
            cap *= 2;
            out->v = xrealloc(out->v, cap * sizeof(*out->v));
        }
        Module *m = &out->v[out->n++];
        m->name = name;
        m->base = start;
        m->size = end - start;
    }

    fclose(f);
    return out->n > 0;
}

void modules_free(Modules *m) {
    for (size_t i = 0; i < m->n; i++) free(m->v[i].name);
    free(m->v);
    m->v = NULL;
    m->n = 0;
}

const Module *modules_get(const Modules *m, const char *name) {
    for (size_t i = 0; i < m->n; i++)
        if (strcmp(m->v[i].name, name) == 0) return &m->v[i];
    return NULL;
}
