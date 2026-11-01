/* Read-only ELKS ktcp status. No DNS, ping, or external connections. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "fetch.h"

/* ktcp's local netconf protocol, independent of its private C headers.
 * NS_GET_CONFIG (6): IPv4, mask, gateway, MAC[6], name[8] = 26 bytes.
 * The three addresses are already in network byte order.
 */
#define CONFIG_BYTES 26
#define QUERY_SECONDS 2

static int config_read(int fd, unsigned char *buf)
{
    unsigned have = 0;
    int n;
    fd_set ready;
    struct timeval wait;
    time_t end = time(NULL) + QUERY_SECONDS, now;
    while (have < CONFIG_BYTES) {
        now = time(NULL);
        if (now >= end) return 0;
        wait.tv_sec = end - now;
        wait.tv_usec = 0;
        FD_ZERO(&ready);
        FD_SET(fd, &ready);
        if (select(fd + 1, &ready, NULL, NULL, &wait) <= 0) return 0;
        n = read(fd, buf + have, CONFIG_BYTES - have);
        if (n <= 0) return 0;
        have += n;
    }
    return 1;
}

static int nonzero(const unsigned char *ip)
{
    return ip[0] || ip[1] || ip[2] || ip[3];
}

static void address_row(struct fetch_info *info, const char *label,
                        const unsigned char *ip)
{
    char text[16];
    sprintf(text, "%u.%u.%u.%u", (unsigned)ip[0], (unsigned)ip[1],
            (unsigned)ip[2], (unsigned)ip[3]);
    fetch_add(info, label, text);
}

void fetch_network(struct fetch_info *info)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    unsigned char ip[4], config[CONFIG_BYTES];
    const unsigned char request[2] = { 6, 0 };
    int fd, have_ip = 0, have_config = 0;
    const char *status = "unavailable";

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        if (errno == ENETDOWN)
            status = "disconnected";
        fetch_add(info, "Network", status);
        return;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) goto done;
    /* Works on older ktcp versions, including HP 200LX release 4.0. */
    if (getsockname(fd, (struct sockaddr *)&addr, &len) == 0 &&
            len >= sizeof(addr) && addr.sin_family == AF_INET) {
        memcpy(ip, &addr.sin_addr.s_addr, sizeof(ip));
        have_ip = 1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2); /* 0.0.0.0:2 is intercepted locally by ktcp. */
    if (in_connect(fd, (struct sockaddr *)&addr, sizeof(addr), QUERY_SECONDS) == 0 &&
            write(fd, request, sizeof(request)) == sizeof(request) &&
            config_read(fd, config)) {
        memcpy(ip, config, sizeof(ip));
        have_ip = have_config = 1;
    }
    if (have_ip)
        status = nonzero(ip) ? "connected (configured)" : "disconnected (no IPv4)";
    else status = "active (details unavailable)";
done:
    close(fd);
    fetch_add(info, "Network", status);
    if (have_ip && nonzero(ip)) {
        address_row(info, "IPv4", ip);
        if (have_config) {
            address_row(info, "Netmask", config + 4);
            if (nonzero(config + 8)) address_row(info, "Gateway", config + 8);
            else fetch_add(info, "Gateway", "none");
        }
    }
    /* netconf's interface name is misleading for SLIP on some versions;
     * do not infer a link type or hardware carrier state from it. */
}
