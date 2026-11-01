/* Deterministic renderer fixture. Never linked into an ELKS binary. */
#include <stdlib.h>
#include "fetch.h"
unsigned fetch_columns(void) { return 80; }
int fetch_is_terminal(void) { return 0; }
void fetch_collect(struct fetch_info *info, const char *model)
{
    fetch_add(info, "OS", "ELKS fixture (not hardware)");
    fetch_add(info, "Kernel", "test");
    fetch_add(info, "Hostname", "fixture");
    fetch_add(info, "Platform", "ibmpc i8086");
    if (model) fetch_add(info, "Model (set)", model);
    fetch_add(info, "User", "fixture");
    fetch_add(info, "Uptime", "0d 1h 2m 3s");
    fetch_add(info, "Main RAM", "123K used, 234K free");
    fetch_add(info, "XMS RAM", "0K used, 1024K free");
    fetch_add(info, "Network", "connected (configured)");
    fetch_add(info, "IPv4", "192.168.100.200");
    fetch_add(info, "Netmask", "255.255.255.0");
    fetch_add(info, "Gateway", "192.168.100.1");
    fetch_add(info, "Shell (env)", "/bin/sh");
    fetch_add(info, "Term (env)", getenv("TERM"));
    fetch_add(info, "TTY", "/dev/tty1");
}
