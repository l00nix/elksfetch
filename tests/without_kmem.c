/* Test helper only: run the real binary in a root with no /dev/kmem. */
#include <unistd.h>
#include <stdio.h>
int main(void)
{
    if (chroot("/tmp/froot") < 0 || chdir("/") < 0) {
        perror("test chroot"); return 1;
    }
    execl("/fetch", "elksfetch", "--plain", (char *)0);
    perror("test exec"); return 1;
}
