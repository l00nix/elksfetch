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
    fetch_add(info, "Main RAM", "123K used, 234K free");
    fetch_add(info, "Term (env)", getenv("TERM"));
}
