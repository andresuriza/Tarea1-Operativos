#include <errno.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "file_metadata.h"
static int send_all(int fd, const void *buf, size_t len) {
    const unsigned char *p = (const unsigned char*)buf;
    size_t left = len;
    while (left) {
        ssize_t n = send(fd, p, left, 0);
        if (n < 0) { if (errno==EINTR) continue; return -1; }
        if (n == 0) { errno = ECONNRESET; return -1; }
        p += n; left -= (size_t)n;
    }
    return 0;
}

static void dump_hex(const unsigned char *p, size_t n) {
    for (size_t i=0;i<n;i++) printf("%02x%s", p[i], i+1<n?" ":"\n");
}

static int recv_exact(int fd, void *buf, size_t n){
    uint8_t *p=buf; size_t left=n;       // puntero al buffer y cuántos bytes faltan
    while(left){
        ssize_t r = recv(fd, p, left, 0); // intenta leer "left" bytes del socket
        if (r < 0){                       // si hubo error
            if(errno==EINTR) continue;    // si fue por señal (interrumpido), reintenta
            return -1;                    // otro error => fallo
        }
        if (r == 0) return -1;            // el peer cerró la conexión antes de leer todo
        p += r; left -= (size_t)r;        // avanzás el puntero y restás lo leído
    }
    return 0; // éxito, ya recibiste exactamente n bytes
}


int send_header_and_name(int sock, const char *filename) {
    uint8_t header[11];
    build_header_v2(header, filename);

    // 1) HEADER
    if (send_all(sock, header, sizeof header) != 0) return -1;
    uint8_t ack;
    if (recv_exact(sock, &ack, 1) != 0 || ack != 0xAA) { errno=EPROTO; return -1; }

    // 2) LEN
    uint16_t net_len = htons((uint16_t)strlen(filename));
    if (send_all(sock, &net_len, 2) != 0) return -1;
    if (recv_exact(sock, &ack, 1) != 0 || ack != 0xAA) { errno=EPROTO; return -1; }

    // 3) NAME
    size_t name_len = strlen(filename);
    if (send_all(sock, filename, name_len) != 0) return -1;
    if (recv_exact(sock, &ack, 1) != 0 || ack != 0xAA) { errno=EPROTO; return -1; }

    return 0;
}