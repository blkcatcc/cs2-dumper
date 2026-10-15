#include "dumper.h"

#include <dirent.h>
#include <unistd.h>

/* Replays every signature in config.json against the installed modules, with no
   process attached. This exercises the same pattern and operation code the live
   dump uses, so a game update can be triaged without starting the game. */

size_t vtables_count_image(const u8 *image, size_t len, u64 base);

static const char *g_game_dir = NULL;

static int find_module(const char *name, char *out, size_t out_sz) {
    static const char *subdirs[] = { "bin/linuxsteamrt64", "csgo/bin/linuxsteamrt64" };
    for (size_t i = 0; i < sizeof(subdirs) / sizeof(subdirs[0]); i++) {
        snprintf(out, out_sz, "%s/%s/%s", g_game_dir, subdirs[i], name);
        if (access(out, R_OK) == 0) return 1;
    }
    return 0;
}

/* Steam libraries from libraryfolders.vdf, plus the usual defaults. */
static int find_game_dir(char *out, size_t out_sz) {
    static const char *bases[] = {
        "~/.local/share/Steam", "~/.steam/steam",
        "~/.var/app/com.valvesoftware.Steam/.local/share/Steam",
    };
    static const char *rel = "steamapps/common/Counter-Strike Global Offensive/game";

    for (size_t i = 0; i < sizeof(bases) / sizeof(bases[0]); i++) {
        const char *b = bases[i];
        /* Bounded so the joined path below cannot truncate. */
        char root[3072];
        if (b[0] == '~') snprintf(root, sizeof(root), "%s%s", getenv("HOME"), b + 1);
        else snprintf(root, sizeof(root), "%s", b);

        snprintf(out, out_sz, "%s/%s", root, rel);
        if (access(out, R_OK) == 0) return 1;

        /* Any library root named in libraryfolders.vdf. */
        char vdf[3584];
        snprintf(vdf, sizeof(vdf), "%s/steamapps/libraryfolders.vdf", root);
        FILE *f = fopen(vdf, "r");
        if (!f) continue;
        char line[4096];
        while (fgets(line, sizeof(line), f)) {
            char *k = strstr(line, "\"path\"");
            if (!k) continue;
            char *q = strchr(k + 6, '"');
            if (!q) continue;
            char *end = strchr(q + 1, '"');
            if (!end) continue;
            *end = 0;
            snprintf(out, out_sz, "%s/%s", q + 1, rel);
            if (access(out, R_OK) == 0) {
                fclose(f);
                return 1;
            }
        }
        fclose(f);
    }
    return 0;
}

/* Replays the operation chain offline. Returns 0 when an operation would read
   outside the image, which happens for `read` on statically-initialised
   pointers (those live in the relocation table, not the file). */
static int replay(const Image *img, u64 *addr, const Sig *sig) {
    for (size_t k = 0; k < sig->nops; k++) {
        const Op *op = &sig->ops[k];
        switch (op->kind) {
        case OP_ADD:
            *addr += op->value;
            break;
        case OP_SUB:
            *addr -= op->value;
            break;
        case OP_RIP: {
            if (*addr + op->offset + 4 > img->len) return 0;
            i32 disp;
            memcpy(&disp, img->data + *addr + op->offset, 4);
            *addr = (u64)((i64)*addr + disp + (i64)op->len);
            break;
        }
        case OP_READ: {
            if (*addr + 8 > img->len) return 0;
            u64 v = 0;
            memcpy(&v, img->data + *addr, 8);
            *addr = v;
            break;
        }
        case OP_SLICE: {
            size_t n = (size_t)(op->end - op->start);
            if (n > 8) n = 8;
            if (*addr + op->start + n > img->len) return 0;
            u8 bytes[8] = { 0 };
            memcpy(bytes, img->data + *addr + op->start, n);
            memcpy(addr, bytes, 8);
            break;
        }
        }
    }
    return 1;
}

int selfcheck_main(const char *game_dir) {
    char dir[4096];
    if (game_dir) {
        snprintf(dir, sizeof(dir), "%s", game_dir);
    } else if (!find_game_dir(dir, sizeof(dir))) {
        fprintf(stderr, "error: could not locate the CS2 install; pass --game-dir\n");
        return 2;
    }
    g_game_dir = dir;

    Config *cfg = config_load();

    printf("%-22s%-44s%12s  %s\n", "module", "signature", "offset", "status");
    printf("%s\n", "--------------------------------------------------------------------------------");

    int missing = 0, checked = 0;

    for (size_t g = 0; g < cfg->ngroups; g++) {
        const SigGroup *sg = &cfg->groups[g];
        char path[4096];
        if (!find_module(sg->module, path, sizeof(path))) {
            printf("%-22s<module not found>\n", sg->module);
            missing += (int)sg->nsigs;
            continue;
        }

        Image img;
        if (!image_load(&img, path)) {
            printf("%-22s<unable to read>\n", sg->module);
            missing += (int)sg->nsigs;
            continue;
        }

        for (size_t s = 0; s < sg->nsigs; s++) {
            const Sig *sig = &sg->sigs[s];
            Pattern pat;
            checked++;
            if (!pattern_parse(&pat, sig->pattern)) {
                printf("%-22s%-44s%12s  PARSE ERROR\n", sg->module, sig->name, "-");
                missing++;
                continue;
            }
            size_t off = 0;
            if (!pattern_scan(&pat, img.data, img.len, &off)) {
                printf("%-22s%-44s%12s  MISSING\n", sg->module, sig->name, "-");
                missing++;
                pattern_free(&pat);
                continue;
            }
            pattern_free(&pat);

            /* img.data is laid out by virtual address, so a scan offset is
               already the address. */
            u64 addr = off;
            if (!replay(&img, &addr, sig)) {
                printf("%-22s%-44s%12s  offline (needs live memory)\n", sg->module,
                       sig->name, "-");
                continue;
            }

            printf("%-22s%-44s%12llu  ok\n", sg->module, sig->name,
                   (unsigned long long)addr);
        }
        image_free(&img);
    }

    printf("%s\n", "--------------------------------------------------------------------------------");
    printf("checked %d signatures, %d unresolved\n", checked, missing);

    /* The vtable scanner runs on the same image data; report what it finds so a
       game update can be triaged offline. */
    printf("\nvtables (offline):\n");
    char libclient[4096];
    if (find_module("libclient.so", libclient, sizeof(libclient))) {
        Image img;
        if (image_load(&img, libclient)) {
            printf("  %-22s %zu classes\n", "libclient.so",
                   vtables_count_image(img.data, img.len, img.base));
            image_free(&img);
        }
    }

    return missing ? 1 : 0;
}
