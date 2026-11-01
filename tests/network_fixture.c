/* Exercise the real collector with deterministic socket replies. */
#include <assert.h>
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

enum { DOWN, ERROR, BIND_FAIL, CONNECT_FAIL, OLD, FULL, PARTIAL, EOF_REPLY,
       NO_IP, NO_GATEWAY, NO_DETAILS, SHORT_WRITE, DEADLINE };
static int scenario, closed, offset, reads, waits;
static const unsigned char reply[26] = {
    10,0,2,15, 255,255,255,0, 10,0,2,2, 0,0,0,0,0,0, 'n','e','0',0,0,0,0,0
};
static int fake_socket(int family, int type, int protocol)
{
    assert(family == AF_INET && type == SOCK_STREAM && protocol == 0);
    if (scenario == DOWN || scenario == ERROR) {
        errno = scenario == DOWN ? ENETDOWN : EMFILE;
        return -1;
    }
    return 3;
}
static int fake_bind(int fd, const struct sockaddr *p, socklen_t len)
{
    const struct sockaddr_in *a = (const struct sockaddr_in *)p;
    assert(fd == 3 && len == sizeof(*a) && a->sin_family == AF_INET);
    assert(a->sin_addr.s_addr == 0 && a->sin_port == 0);
    return scenario == BIND_FAIL ? -1 : 0;
}
static int fake_getsockname(int fd, struct sockaddr *p, socklen_t *len)
{
    struct sockaddr_in *a = (struct sockaddr_in *)p;
    assert(fd == 3 && *len == sizeof(*a));
    if (scenario == NO_DETAILS) return -1;
    memcpy(&a->sin_addr.s_addr, reply, 4);
    if (scenario == NO_IP) a->sin_addr.s_addr = 0;
    return 0;
}
static int fake_connect(int fd, const struct sockaddr *p, socklen_t len, int seconds)
{
    const struct sockaddr_in *a = (const struct sockaddr_in *)p;
    assert(fd == 3 && len == sizeof(*a) && seconds == 2);
    assert(a->sin_family == AF_INET && a->sin_addr.s_addr == 0 && ntohs(a->sin_port) == 2);
    return scenario == CONNECT_FAIL ? -1 : 0;
}
static ssize_t fake_write(int fd, const void *buf, size_t len)
{
    const unsigned char *b = buf;
    assert(fd == 3 && len == 2 && b[0] == 6 && b[1] == 0);
    return scenario == SHORT_WRITE ? 1 : 2;
}
static int fake_select(int nfds, fd_set *r, fd_set *w, fd_set *e, struct timeval *t)
{
    assert(nfds == 4 && FD_ISSET(3, r) && !w && !e);
    assert(t->tv_sec > 0 && t->tv_sec <= 2);
    ++waits;
    return scenario == OLD || scenario == NO_DETAILS ? 0 : 1;
}
static ssize_t fake_read(int fd, void *buf, size_t len)
{
    unsigned char bytes[26];
    size_t n = len;
    assert(fd == 3 && len <= sizeof(bytes));
    ++reads;
    if (scenario == EOF_REPLY) return 0;
    memcpy(bytes, reply, sizeof(bytes));
    if (scenario == NO_IP) memset(bytes, 0, 4);
    if (scenario == NO_GATEWAY) memset(bytes + 8, 0, 4);
    if ((scenario == PARTIAL || scenario == DEADLINE) && n > 3) n = 3;
    memcpy(buf, bytes + offset, n);
    offset += n;
    return (ssize_t)n;
}
static int fake_close(int fd) { assert(fd == 3); ++closed; return 0; }
static time_t fake_time(time_t *p)
{
    time_t now = 100 + (scenario == DEADLINE ? reads : 0);
    if (p) *p = now;
    return now;
}
void fetch_add(struct fetch_info *info, const char *label, const char *value)
{
    assert(info->count < FETCH_ROWS && strlen(value) < FETCH_TEXT);
    info->rows[info->count].label = label;
    strcpy(info->rows[info->count++].value, value);
}
#define socket fake_socket
#define bind fake_bind
#define getsockname fake_getsockname
#define in_connect fake_connect
#define write fake_write
#define select fake_select
#define read fake_read
#define close fake_close
#define time fake_time
#include "../platform/network.c"

int main(void)
{
    struct fetch_info info;
    for (scenario = DOWN; scenario <= DEADLINE; ++scenario) {
        memset(&info, 0, sizeof(info));
        closed = offset = reads = waits = 0;
        fetch_network(&info);
        assert(!strcmp(info.rows[0].label, "Network"));
        assert(closed == (scenario != DOWN && scenario != ERROR));
        if (scenario == DOWN) {
            assert(info.count == 1 && !strcmp(info.rows[0].value, "disconnected"));
        } else if (scenario == ERROR || scenario == BIND_FAIL) {
            assert(info.count == 1 && !strcmp(info.rows[0].value, "unavailable"));
        } else if (scenario == NO_DETAILS) {
            assert(info.count == 1 && !strcmp(info.rows[0].value, "active (details unavailable)"));
        } else if (scenario == NO_IP) {
            assert(info.count == 1 && !strcmp(info.rows[0].value, "disconnected (no IPv4)"));
        } else {
            assert(!strcmp(info.rows[0].value, "connected (configured)"));
            assert(!strcmp(info.rows[1].label, "IPv4"));
            assert(!strcmp(info.rows[1].value, "10.0.2.15"));
            if (scenario == FULL || scenario == PARTIAL || scenario == NO_GATEWAY) {
                assert(info.count == 4);
                assert(!strcmp(info.rows[2].value, "255.255.255.0"));
                assert(!strcmp(info.rows[3].value, scenario == NO_GATEWAY ? "none" : "10.0.2.2"));
            } else assert(info.count == 2);
        }
        if (scenario == DEADLINE) assert(reads == 2 && waits == 2);
    }
    puts("13 network scenarios passed");
    return 0;
}
