#include <errno.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "file_metadata.h"
#include <unistd.h>
#include "wire.h"

#define CHUNK (64*1024u)
int send_file_data(int sock, const char *filename, uint64_t *filesize);

int send_all(int fd, const void *buf, size_t len) {
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

int recv_exact(int fd, void *buf, size_t n){
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


int send_data(int sock, const char *filename) {
    uint8_t header[11];
    build_header_v2(header, filename);

    // 1) HEADER
    if (send_all(sock, header, sizeof header) != 0) return -1;
    uint8_t ack;
    if (recv_exact(sock, &ack, 1) != 0 || ack != 0xAA) { errno=EPROTO; return -1; }

    // 2) NAME
    size_t name_len = strlen(filename);
    if (send_all(sock, filename, name_len) != 0) return -1;
    if (recv_exact(sock, &ack, 1) != 0 || ack != 0xAA) { errno=EPROTO; return -1; }
    // 3) FILE DATA
    uint64_t fsize_net;
    memcpy(&fsize_net, header + 2, sizeof(uint64_t));
    uint64_t fsize = ntohll_u64(fsize_net);
    if (send_file_data(sock, filename, &fsize) != 0) return -1;
    return 0;
}
int send_exit(int sock) {
    uint8_t header[11] = {0}; // todos ceros
    header[10] = OP_END;      // posición del file_type
    if (send_all(sock, header, sizeof header) != 0) return -1;

    uint8_t ack;
    if (recv_exact(sock, &ack, 1) != 0 || ack != 0xAA) {
        errno = EPROTO;
        return -1;
    }
    return 0;
}


int send_file_data(int sock, const char *filename, uint64_t *filesize){
    char fullpath[512];
    snprintf(fullpath, sizeof fullpath, "/data/%s", filename);
    int fd = open(fullpath, O_RDONLY);
    if(fd < 0) {
        perror("open");
        return -1;
    }
    uint64_t offset = 0;
    if(lseek(fd,0, SEEK_SET)< 0){
        perror("lseek start");
        close(fd);
        return -1;
    }
    unsigned char buffer[CHUNK];
    while (offset < *filesize){
        size_t to_read = (size_t)((*filesize - offset) < CHUNK ? (*filesize - offset) : CHUNK); 
        ssize_t r = read(fd, buffer, to_read);
        if (r < 0) {
            if(errno == EINTR) continue;
            perror("read");
            close(fd);
            return -1;
        }
        if(r==0){
            fprintf(stderr, "EOF inesperado\n");
            close(fd);
            return -1;
        }
        if(send_all(sock, buffer, (size_t)r) != 0){
            perror("send chunk");
            close(fd);
            return -1;
        }
        ack_t ack;
        if(recv_exact(sock, &ack, sizeof ack) != 0){
            perror("recv ack");
            close(fd);
            return -1;
        }
        if (ack.code != ACK_CODE_CNTN) {
            errno = EPROTO;
            fprintf(stderr, "ACK code invalido: 0x%02x\n", ack.code);
            close(fd);
            return -1;
        }
        uint64_t next = ntohll_u64(ack.next_offset);
        uint64_t sent_end = offset + (uint64_t)r;
        if (next > *filesize || next < offset || next > sent_end) {
            fprintf(stderr, "ACK next_offset fuera de rango (offset=%llu, sent_end=%llu, next=%llu)\n",
                    (unsigned long long)offset, (unsigned long long)sent_end, (unsigned long long)next);
            close(fd); return -1;
        }

        // 6) si el server ancló menos, nos “devolvemos”
        if (next != sent_end) {
            if (lseek(fd, (off_t)next, SEEK_SET) < 0) { perror("lseek back"); close(fd); return -1; }
        }

        offset = next;
    }
    close(fd);
    return 0;


}