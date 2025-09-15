#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include "handle_client.h"
#include "socket_helpers.h"
#include <string.h>
#include <sys/stat.h>
int recv_header(int cfd, header_v2_t *out) {
    uint8_t hdr[11];
    if (recv_exact(cfd, hdr, sizeof hdr) != 0) return -1;

    out->name_len = ((uint16_t)hdr[0] << 8) | hdr[1];
    if (out->name_len == 0 || out->name_len > 512) return -1;

    uint64_t raw;
    memcpy(&raw, &hdr[2], 8);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    out->file_size = __builtin_bswap64(raw);
#else
    out->file_size = raw;
#endif
    out->file_type = hdr[10];
    return 0;
}

int recv_filename(int cfd, char *out, size_t maxlen, uint16_t name_len) {
    if (name_len >= maxlen) return -1;
    if (recv_exact(cfd, out, name_len) != 0) return -1;
    out[name_len] = '\0';
    return 0;
}

const char* file_type_str(uint8_t file_type) {
    switch (file_type) {
        case 0: return "jpg";
        case 1: return "png";
        case 2: return "gif";
        default: return "otros";
    }
}

static int ensure_dir(const char *p, mode_t mode) {
    if (mkdir(p, mode) == 0) return 0;
    if (errno == EEXIST)    return 0;
    perror("[srv] mkdir");
    return -1;
}

int prepare_output_path(char *out_path, size_t cap,
                        const char *namebuf, const char *type_str) {
    // 1) asegurar raíz
    if (ensure_dir("uploads", 0755) != 0) return -1;

    // 2) asegurar subcarpeta por tipo
    char folder[256];
    snprintf(folder, sizeof folder, "uploads/%s", type_str);
    if (ensure_dir(folder, 0755) != 0) return -1;

    // 3) construir path final
    snprintf(out_path, cap, "%s/%s", folder, namebuf);
    return 0;
}

int send_ack(int cfd) {
    uint8_t ack = 0xAA;
    return send_all(cfd, &ack, 1);
}

int open_and_truncate(const char *path, uint64_t file_size) {
    int fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0) return -1;
    if (ftruncate(fd, file_size) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}
// Endianness helper
static inline uint64_t htobe64_u64(uint64_t x){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(x);
#else
    return x;
#endif
}

// Envía ACK de "continuar" con el offset acumulado (en big-endian)
int send_ack_cntn(int cfd, uint64_t next_off_host){
    ack_t a;
    a.code = ACK_CODE_CNTN;
    a.next_offset = htobe64_u64(next_off_host);
    return send_all(cfd, &a, sizeof a);
}

// Escribe exactamente n bytes (maneja escrituras parciales)
static int write_full(int fd, const uint8_t *buf, size_t n){
    size_t off = 0;
    while (off < n){
        ssize_t w = write(fd, buf + off, n - off);
        if (w < 0){
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)w;
    }
    return 0;
}

// Recibe el archivo en chunks de 64 KiB (último puede ser menor).
// Tras cada chunk persistido, envía ACK_CODE_CNTN con next_offset acumulado.
int receive_file_data(int cfd, int outfd, uint64_t file_size){
    uint8_t *buf = malloc(CHUNK_SIZE);
    if (!buf) return -1;

    uint64_t offset = 0;
    while (offset < file_size){
        size_t target = (size_t)((file_size - offset) < CHUNK_SIZE ? (file_size - offset) : CHUNK_SIZE);

        // Acumular exactamente 'target' bytes antes de ACK (alineado al chunk del cliente)
        size_t got = 0;
        while (got < target){
            ssize_t r = recv(cfd, buf + got, target - got, 0);
            if (r == 0){ free(buf); errno = ECONNRESET; return -1; }
            if (r < 0){
                if (errno == EINTR) continue;
                free(buf);
                return -1;
            }
            got += (size_t)r;
        }

        // Persistir el bloque recibido (maneja escrituras parciales)
        if (write_full(outfd, buf, got) != 0){
            free(buf);
            return -1;
        }

        offset += got;

        // ACK con el total persistido hasta ahora (big-endian en el wire)
        if (send_ack_cntn(cfd, offset) != 0){
            free(buf);
            return -1;
        }
    }

    fsync(outfd);
    free(buf);
    return 0;
}


void* handle_client(void *arg) {
    int cfd = (int)(intptr_t)arg;

    // Asegurar modo bloqueante
    int fl = fcntl(cfd, F_GETFL, 0);
    if (fl & O_NONBLOCK) fcntl(cfd, F_SETFL, fl & ~O_NONBLOCK);

    // 1. Header
    header_v2_t hdr;
    if (recv_header(cfd, &hdr) != 0) { close(cfd); return NULL; }
    printf("[srv] header recibido: name_len=%u, file_size=%llu, type=0x%02X\n",
        hdr.name_len, (unsigned long long)hdr.file_size, hdr.file_type);
    if (send_ack(cfd) != 0) { close(cfd); return NULL; }

    // 2. Nombre de archivo
    char namebuf[513];
    if (recv_filename(cfd, namebuf, sizeof namebuf, hdr.name_len) != 0) {
        perror("[srv] recv filename");
        close(cfd); return NULL;
    }
    printf("[srv] nombre de archivo: %s\n", namebuf);
    if (send_ack(cfd) != 0) { close(cfd); return NULL; }

    // 3. Ruta de salida
    const char *type_str = file_type_str(hdr.file_type);
    char path[512];
    if (prepare_output_path(path, sizeof path, namebuf, type_str) != 0) {
        close(cfd);
        return NULL;
    }

    // 4. Crear y truncar
    int outfd = open_and_truncate(path, hdr.file_size);
    if (outfd < 0) {
        perror("[srv] open/truncate");
        close(cfd); return NULL;
    }

    // (Todavía no se recibe el contenido, se hace después)

    printf("[srv] ACK por nombre enviado y archivo preparado\n");
    if (receive_file_data(cfd, outfd, hdr.file_size) != 0){
        perror("[srv] receive_file_data");
        close(outfd);
        close(cfd);
        return NULL;
    }

    printf("[srv] archivo completo: %s (%llu bytes)\n", path, (unsigned long long)hdr.file_size);
    close(outfd);
    close(cfd);
    return NULL;
}
