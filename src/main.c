#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch.h"

void fetch_add(struct fetch_info *info, const char *label, const char *value)
{
    unsigned i = 0;
    struct fetch_row *row;
    if (info->count == FETCH_ROWS || !value || !*value) return;
    row = &info->rows[info->count++];
    row->label = label;
    while (value[i] && i < FETCH_TEXT - 1) {
        unsigned char c = (unsigned char)value[i];
        row->value[i++] = (c >= 32 && c <= 126) ? c : '?';
    }
    row->value[i] = 0;
}

static void usage(void)
{
    puts("elksfetch " FETCH_VERSION " - a small system portrait for ELKS\n"
         "Usage: elksfetch [options]\n"
         "  --logo small|wide|none   Logo style (default: small)\n"
         "  --compact               Information only\n"
         "  --plain                 Information only, no colour\n"
         "  --mono                  Disable colour, keep logo\n"
         "  --color auto|always|never\n"
         "  --width N               Output columns, 20..240\n"
         "  --host NAME             Explicit machine label (marked as set)\n"
         "  --help                  Show help\n"
         "  --version               Show version");
}

int main(int argc, char **argv)
{
    static struct fetch_info info;
    const char *model = NULL, *term;
    unsigned width = 0;
    int logo = 1, color = -1, i;
    for (i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help")) { usage(); return 0; }
        if (!strcmp(arg, "--version")) { puts("elksfetch " FETCH_VERSION); return 0; }
        if (!strcmp(arg, "--compact")) { logo = 0; continue; }
        if (!strcmp(arg, "--plain")) { logo = 0; color = 0; continue; }
        if (!strcmp(arg, "--mono")) { color = 0; continue; }
        if (i + 1 == argc) goto bad_option;
        if (!strcmp(arg, "--host")) {
            model = argv[++i];
            if (!*model) goto bad_option;
        } else if (!strcmp(arg, "--width")) {
            const char *p = argv[++i];
            unsigned n = 0;
            if (!*p) goto bad_option;
            while (*p) {
                if (*p < '0' || *p > '9' || n > 240) goto bad_option;
                n = n * 10 + (unsigned)(*p++ - '0');
            }
            if (n < 20 || n > 240) goto bad_option;
            width = n;
        } else if (!strcmp(arg, "--logo")) {
            const char *p = argv[++i];
            if (!strcmp(p, "small")) logo = 1;
            else if (!strcmp(p, "wide")) logo = 2;
            else if (!strcmp(p, "none")) logo = 0;
            else goto bad_option;
        } else if (!strcmp(arg, "--color")) {
            const char *p = argv[++i];
            if (!strcmp(p, "auto")) color = -1;
            else if (!strcmp(p, "always")) color = 1;
            else if (!strcmp(p, "never")) color = 0;
            else goto bad_option;
        } else goto bad_option;
    }
    if (!width) width = fetch_columns();
    term = getenv("TERM");
    if (color < 0)
        color = fetch_is_terminal() && term && *term && strcmp(term, "dumb") && !getenv("NO_COLOR");
    fetch_collect(&info, model);
    fetch_render(&info, width, logo, color);
    return ferror(stdout) ? 1 : 0;
bad_option:
    fputs("elksfetch: invalid or missing option value; see --help\n", stderr);
    return 2;
}
