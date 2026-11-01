#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <linuxmt/mem.h>
#include <arch/param.h>
#include "fetch.h"

int fetch_is_terminal(void) { return isatty(STDOUT_FILENO); }

unsigned fetch_columns(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col >= 20 && ws.ws_col <= 240)
        return ws.ws_col;
    return 80;
}

static int ticks_read(int fd, unsigned ds, unsigned off, unsigned long *ticks)
{
    /* O_ALT uses a segment:offset position, as in ELKS ps/meminfo. */
    long pos = (long)(((unsigned long)ds << 16) | off);
    return lseek(fd, pos, SEEK_SET) == pos && read(fd, ticks, sizeof(*ticks)) == sizeof(*ticks);
}

static void memory_and_uptime(struct fetch_info *info)
{
    struct mem_usage mu;
    unsigned ds, off, tries;
    unsigned long a, b, seconds;
    char text[FETCH_TEXT];
    int fd = open("/dev/kmem", O_RDONLY | O_ALT), have_up = 0;
    if (fd < 0) {
        fetch_add(info, "Uptime", "unavailable (/dev/kmem)");
        fetch_add(info, "Main RAM", "unavailable (/dev/kmem)");
        return;
    }
    if (!ioctl(fd, MEM_GETDS, &ds) && !ioctl(fd, MEM_GETUPTIME, &off)) {
        /* Retry a changing/torn 32-bit read without disabling interrupts. */
        for (tries = 0; tries < 3; ++tries) {
            if (!ticks_read(fd, ds, off, &a) || !ticks_read(fd, ds, off, &b)) break;
            if (b - a <= HZ) { have_up = 1; break; }
        }
    }
    if (have_up) {
        seconds = b / HZ;
        sprintf(text, "%lud %luh %lum %lus", seconds / 86400UL,
                (seconds / 3600UL) % 24, (seconds / 60UL) % 60, seconds % 60);
        fetch_add(info, "Uptime", text);
    } else fetch_add(info, "Uptime", "unavailable");
    if (!ioctl(fd, MEM_GETUSAGE, &mu)) {
        sprintf(text, "%uK used, %uK free", mu.main_used, mu.main_free);
        fetch_add(info, "Main RAM", text);
        if (mu.xms_used || mu.xms_free) {
            sprintf(text, "%uK used, %uK free", mu.xms_used, mu.xms_free);
            fetch_add(info, "XMS RAM", text);
        }
    } else fetch_add(info, "Main RAM", "unavailable");
    close(fd);
}

void fetch_collect(struct fetch_info *info, const char *model)
{
    struct utsname u;
    struct passwd *pw;
    char text[FETCH_TEXT];
    const char *value;
    fetch_add(info, "OS", "ELKS");
    if (uname(&u) == 0) {
        u.nodename[sizeof(u.nodename)-1] = 0;
        u.release[sizeof(u.release)-1] = 0;
        u.machine[sizeof(u.machine)-1] = 0;
        fetch_add(info, "Kernel", u.release);
        fetch_add(info, "Hostname", u.nodename);
        fetch_add(info, "Platform", u.machine);
    } else fetch_add(info, "Kernel", "unavailable");
    if (model) fetch_add(info, "Model (set)", model);
    pw = getpwuid(getuid());
    if (pw) fetch_add(info, "User", pw->pw_name);
    else {
        sprintf(text, "uid %u", (unsigned)getuid());
        fetch_add(info, "User", text);
    }
    memory_and_uptime(info);
    fetch_network(info);
    value = getenv("SHELL");
    fetch_add(info, "Shell (env)", value ? value : "not set");
    value = getenv("TERM");
    fetch_add(info, "Term (env)", value ? value : "not set");
    value = ttyname(STDIN_FILENO);
    if (value) fetch_add(info, "TTY", value);
}
