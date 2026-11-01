#include <stdio.h>
#include <string.h>
#include "fetch.h"
#include "wide-logo.h"

static const char *small[] = {
    "EEEEEE LL    KK  KK  SSSSS",
    "EE     LL    KK KK  SS    ",
    "EEEEE  LL    KKKK    SSSS ",
    "EE     LL    KK KK      SS",
    "EEEEEE LLLLL KK  KK SSSSS ",
    "                         ",
    "   Linux on sixteen bits "
};

/* Count columns ourselves: no cursor addressing, UTF-8, or terminal library. */
static unsigned clipped(const char *s, unsigned room)
{
    unsigned n = 0;
    while (*s && n < room) { putchar(*s++); ++n; }
    return n;
}

static void row(const struct fetch_row *r, unsigned room, int color)
{
    unsigned used;
    if (color) fputs("\033[1;33m", stdout);
    used = clipped(r->label, room);
    if (color) fputs("\033[0m", stdout);
    if (used < room) { putchar(':'); ++used; }
    if (used < room) { putchar(' '); ++used; }
    clipped(r->value, room - used);
}

void fetch_render(const struct fetch_info *info, unsigned width, int logo, int color)
{
    unsigned i, n, used, rows;
    /* Reserve the last column to avoid terminal auto-wrap on CR/LF. */
    unsigned room = width > 1 ? width - 1 : 1;
    if (logo == 2) {
        if (room >= 100) {
            if (color) fputs("\033[1;33m", stdout);
            for (i = 0; i < sizeof(wide_logo)/sizeof(wide_logo[0]); ++i) {
                clipped(wide_logo[i], room); putchar('\n');
            }
            if (color) fputs("\033[0m", stdout);
            putchar('\n');
            logo = 0;
        } else logo = 1;
    }
    if (logo && room >= 64) {
        n = sizeof(small)/sizeof(small[0]);
        rows = info->count > n ? info->count : n;
        putchar('\n');
        for (i = 0; i < rows; ++i) {
            used = 0;
            if (i < n) {
                if (color) fputs("\033[1;33m", stdout);
                used = clipped(small[i], 26);
                if (color) fputs("\033[0m", stdout);
            }
            while (used++ < 29) putchar(' ');
            if (i < info->count) row(&info->rows[i], room - 29, color);
            putchar('\n');
        }
        putchar('\n');
    } else {
        if (logo) { puts("ELKS"); putchar('\n'); }
        for (i = 0; i < info->count; ++i) {
            row(&info->rows[i], room, color); putchar('\n');
        }
    }
}
