#include "dumper.h"

#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static const char *g_out_dir = "output";
static int g_indent = 4;
static int g_verbose = 0;

static void warn(const char *fmt, ...) {
    if (g_verbose < 1) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void info(const char *fmt, ...) {
    if (g_verbose < 2) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

/* ------------------------------------------------------------- retries */

/* The game churns its memory map, so enumeration can transiently come back
   short; require the core modules and retry. */
#define LOOKUP_ATTEMPTS 8
#define LOOKUP_DELAY_MS 50

static int modules_enum_retry(pid_t pid, Modules *out) {
    static const char *required[] = { "libclient.so", "libschemasystem.so",
                                      "libengine2.so", "libserver.so" };
    for (int attempt = 0; attempt < LOOKUP_ATTEMPTS; attempt++) {
        if (!modules_enum(pid, out)) {
            usleep(LOOKUP_DELAY_MS * 1000);
            continue;
        }
        int missing = 0;
        for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); i++)
            if (!modules_get(out, required[i])) missing = 1;
        if (!missing) return 1;

        warn("warning: module list incomplete (%zu modules) - retrying\n", out->n);
        modules_free(out);
        usleep(LOOKUP_DELAY_MS * 1000);
    }
    return 0;
}

static char *build_header(u32 build_number) {
    char *s = xmalloc(32);
    if (build_number) snprintf(s, 32, "// build %u\n\n", build_number);
    else s[0] = 0;
    return s;
}

static void usage(FILE *f) {
    fprintf(f,
            "usage: linux-dumper [options]\n"
            "\n"
            "  -o, --output DIR          output directory (default: output)\n"
            "  -i, --indent-size N       spaces per indent level (default: 4)\n"
            "  -f, --file-types LIST     cs,hpp,json,rs,zig (default: all)\n"
            "  -p, --process-name NAME   process name (default: config.json executable)\n"
            "  -n, --no-log-file         do not write linux-dumper.log\n"
            "  -v, --verbose             increase verbosity, repeatable\n"
            "      --selfcheck           check signatures against an install, no attach\n"
            "      --game-dir DIR        game directory for --selfcheck\n"
            "      --selftest            check the output format, no game needed\n"
            "  -h, --help                show this help\n");
}

int selfcheck_main(const char *game_dir);
int selftest_main(void);

int main(int argc, char **argv) {
    const char *file_types = "cs,hpp,json,rs,zig";
    const char *process_name = NULL;
    int no_log = 0;
    const char *selfcheck_dir = NULL;
    int do_selfcheck = 0;

    static const struct option opts[] = {
        { "output", required_argument, NULL, 'o' },
        { "indent-size", required_argument, NULL, 'i' },
        { "file-types", required_argument, NULL, 'f' },
        { "process-name", required_argument, NULL, 'p' },
        { "no-log-file", no_argument, NULL, 'n' },
        { "verbose", no_argument, NULL, 'v' },
        { "selfcheck", no_argument, NULL, 1 },
        { "game-dir", required_argument, NULL, 2 },
        { "selftest", no_argument, NULL, 3 },
        { "help", no_argument, NULL, 'h' },
        { NULL, 0, NULL, 0 },
    };

    int c;
    while ((c = getopt_long(argc, argv, "o:i:f:p:nvh", opts, NULL)) != -1) {
        switch (c) {
        case 'o': g_out_dir = optarg; break;
        case 'i': g_indent = atoi(optarg); break;
        case 'f': file_types = optarg; break;
        case 'p': process_name = optarg; break;
        case 'n': no_log = 1; break;
        case 'v': g_verbose++; break;
        case 1: do_selfcheck = 1; break;
        case 2: selfcheck_dir = optarg; break;
        case 3: return selftest_main();
        case 'h': usage(stdout); return 0;
        default: usage(stderr); return 2;
        }
    }

    if (do_selfcheck) return selfcheck_main(selfcheck_dir);

    Config *cfg = config_load();

    FileTypes types;
    types.v = xcalloc(16, sizeof(*types.v));
    types.n = 0;
    {
        char *copy = xstrdup(file_types);
        for (char *tok = strtok(copy, ","); tok && types.n < 16; tok = strtok(NULL, ","))
            types.v[types.n++] = xstrdup(tok);
        free(copy);
    }

    const char *name = process_name ? process_name : cfg->executable;
    pid_t pid = proc_find(name);
    if (pid < 0) {
        fprintf(stderr, "error: process \"%s\" not found\n", name);
        return 1;
    }
    info("info: attached to %s (pid %d)\n", name, (int)pid);

    Mem mem;
    if (!mem_open(&mem, pid)) {
        fprintf(stderr, "error: unable to open /proc/%d/mem: %s\n", (int)pid,
                strerror(errno));
        fprintf(stderr, "hint: try running with sudo, or check ptrace_scope\n");
        return 1;
    }

    Modules mods;
    if (!modules_enum_retry(pid, &mods)) {
        fprintf(stderr, "error: unable to enumerate modules\n");
        mem_close(&mem);
        return 1;
    }
    info("info: found %zu modules\n", mods.n);

    mkdir(g_out_dir, 0755);

    NVMap offsets = analyze_offsets(&mem, &mods, cfg);
    size_t n_offsets = 0;
    for (size_t i = 0; i < offsets.n; i++) n_offsets += offsets.v[i].n;
    info("info: found %zu offsets across %zu modules\n", n_offsets, offsets.n);

    NVMap buttons = analyze_buttons(&mem, &mods);
    size_t n_buttons = 0;
    for (size_t i = 0; i < buttons.n; i++) n_buttons += buttons.v[i].n;
    info("info: found %zu buttons\n", n_buttons);

    NVMap interfaces = analyze_interfaces(&mem, &mods);
    size_t n_ifaces = 0;
    for (size_t i = 0; i < interfaces.n; i++) n_ifaces += interfaces.v[i].n;
    info("info: found %zu interfaces across %zu modules\n", n_ifaces, interfaces.n);

    SchemaMap schemas = analyze_schemas(&mem, &mods);
    size_t n_classes = 0, n_enums = 0;
    for (size_t i = 0; i < schemas.n; i++) {
        n_classes += schemas.v[i].nclasses;
        n_enums += schemas.v[i].nenums;
    }
    info("info: found %zu classes and %zu enums across %zu modules\n", n_classes, n_enums,
         schemas.n);

    VtMap vtables = analyze_vtables(&mem, &mods);

    /* Read the build number before writing so every file can carry it. */
    u32 build_number = analyze_build_number(&mem, &mods, &offsets);
    if (build_number) info("info: build number %u\n", build_number);
    else warn("warning: failed to read the build number\n");
    g_banner = build_header(build_number);

    write_nv_map(g_out_dir, "buttons", &buttons, &types, g_indent);
    write_nv_map(g_out_dir, "interfaces", &interfaces, &types, g_indent);
    write_nv_map(g_out_dir, "offsets", &offsets, &types, g_indent);
    for (size_t i = 0; i < schemas.n; i++)
        write_schema(g_out_dir, schemas.v[i].module, &schemas.v[i], &types, g_indent);
    write_vtables_json(g_out_dir, &vtables);
    write_info_json(g_out_dir, build_number);

    info("info: wrote output to %s\n", g_out_dir);

    if (!no_log) {
        FILE *f = fopen("linux-dumper.log", "w");
        if (f) {
            fprintf(f, "build %u\noffsets %zu\nbuttons %zu\ninterfaces %zu\n"
                       "classes %zu\nenums %zu\n",
                    build_number, n_offsets, n_buttons, n_ifaces, n_classes, n_enums);
            fclose(f);
        }
    }

    mem_close(&mem);
    return 0;
}
