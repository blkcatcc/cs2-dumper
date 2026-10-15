#include "dumper.h"

/* Every generated file opens with the build number, when one was read. */
const char *g_banner = "";

void output_begin(Buf *b) {
    buf_puts(b, g_banner);
}

int output_wants(const FileTypes *t, const char *ext) {
    for (size_t i = 0; i < t->n; i++)
        if (strcmp(t->v[i], ext) == 0) return 1;
    return 0;
}
