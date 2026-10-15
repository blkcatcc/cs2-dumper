#include "dumper.h"

#include <time.h>

/* Vtables and info.json are JSON only: their consumers are diffing and slot
   derivation, not code generation. */

void write_vtables_json(const char *dir, const VtMap *map) {
    Buf b;
    buf_init(&b);
    buf_puts(&b, "{\n");
    for (size_t g = 0; g < map->n; g++) {
        const VtGroup *grp = &map->v[g];
        buf_puts(&b, "  \"");
        json_escape(&b, grp->module);
        buf_puts(&b, "\": {\n");
        for (size_t i = 0; i < grp->n; i++) {
            const VtEntry *e = &grp->v[i];
            buf_puts(&b, "    \"");
            json_escape(&b, e->name);
            buf_printf(&b, "\": {\n      \"offset_to_top\": %lld,\n",
                       (long long)(i64)e->vt.offset_to_top);
            buf_printf(&b, "      \"primary\": %llu,\n",
                       (unsigned long long)e->vt.primary);
            buf_printf(&b, "      \"secondary_tables\": %llu,\n",
                       (unsigned long long)e->vt.secondary_tables);
            buf_puts(&b, "      \"slots\": [");
            for (size_t k = 0; k < e->vt.nslots; k++) {
                buf_printf(&b, "%s\n        %llu", k ? "," : "",
                           (unsigned long long)e->vt.slots[k]);
            }
            buf_printf(&b, "%s\n      ]\n", e->vt.nslots ? "" : "");
            buf_printf(&b, "    }%s\n", i + 1 < grp->n ? "," : "");
        }
        buf_printf(&b, "  }%s\n", g + 1 < map->n ? "," : "");
    }
    buf_puts(&b, "}\n");
    write_file(dir, "vtables", "json", &b);
    free(b.p);
}

/* RFC3339 with nanoseconds, matching the reference dump's info.json. */
static void timestamp(char *out, size_t n) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm;
    gmtime_r(&ts.tv_sec, &tm);
    snprintf(out, n, "%04d-%02d-%02dT%02d:%02d:%02d.%09ld+00:00", tm.tm_year + 1900,
             tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
}

void write_info_json(const char *dir, u32 build_number) {
    char ts[64];
    timestamp(ts, sizeof(ts));

    Buf b;
    buf_init(&b);
    buf_puts(&b, "{\n");
    if (build_number)
        buf_printf(&b, "  \"build_number\": %u,\n", build_number);
    else
        buf_puts(&b, "  \"build_number\": null,\n");
    buf_printf(&b, "  \"timestamp\": \"%s\"\n}\n", ts);

    write_file(dir, "info", "json", &b);
    free(b.p);
}
