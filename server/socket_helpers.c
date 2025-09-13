#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
int recv_exact(int fd, void *buf, size_t n){
    uint8_t *p = buf; size_t left = n;
    while (left){
        ssize_t r = recv(fd, p, left, 0);
        if (r < 0){ if (errno == EINTR) continue; return -1; }
        if (r == 0) return -1;
        p += r; left -= (size_t)r;
    }
    return 0;
}
int send_all(int fd, const void *buf, size_t n){
    const uint8_t *p = buf; size_t left = n;
    while (left){
        ssize_t s = send(fd, p, left, 0);
        if (s < 0){ if (errno == EINTR) continue; return -1; }
        if (s == 0){ errno = ECONNRESET; return -1; }
        p += s; left -= (size_t)s;
    }
    return 0;
}