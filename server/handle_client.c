#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "handle_client.h"
#include "socket_helpers.h"
#include "pqueue.h"
#include <stdlib.h>  // por malloc/free (si no lo tenías)
#include "ConfigFunctions.h"  // por Get_Dircolores y Get_Dirhisto

// ===== Definiciones WIRE locales (copiadas del cliente) =====
#define ACK_CODE_OK   0xAA
#define ACK_CODE_CNTN 0x55
#define OP_END        0xFF


#ifndef CHUNK_SIZE
#define CHUNK_SIZE (64*1024u)
#endif

#ifndef OP_END
#define OP_END 0xFF
#endif

// ===== Helpers de endian =====
static inline uint64_t htobe64_u64(uint64_t x){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(x);
#else
    return x;
#endif
}

// ===== Lógica existente (con ajustes mínimos) =====

int recv_header(int cfd, header_v2_t *out) {
    uint8_t hdr[11];
    if (recv_exact(cfd, hdr, sizeof hdr) != 0) return -1;

    uint16_t name_len = ((uint16_t)hdr[0] << 8) | hdr[1];

    uint64_t raw;
    memcpy(&raw, &hdr[2], 8);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t file_size = __builtin_bswap64(raw);
#else
    uint64_t file_size = raw;
#endif

    uint8_t file_type = hdr[10];

    // Permitir END (sin nombre ni tamaño)
    if (file_type == OP_END) {
        out->name_len = 0;
        out->file_size = 0;
        out->file_type = OP_END;
        return 0;
    }

    // Validación normal
    if (name_len == 0 || name_len > 512) return -1;

    out->name_len = name_len;
    out->file_size = file_size;
    out->file_type = file_type;
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
    if (ensure_dir("uploads", 0755) != 0) return -1;

    char folder[256];
    snprintf(folder, sizeof folder, "uploads/%s", type_str);
    if (ensure_dir(folder, 0755) != 0) return -1;

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
    if (ftruncate(fd, (off_t)file_size) != 0) {
        close(fd);
        return -1;
    }
    return fd;
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

// Recibe el archivo en chunks de 64 KiB y ACKea con offset acumulado
int receive_file_data(int cfd, int outfd, uint64_t file_size){
    uint8_t *buf = malloc(CHUNK_SIZE);
    if (!buf) return -1;

    uint64_t offset = 0;
    while (offset < file_size){
        size_t target = (size_t)((file_size - offset) < CHUNK_SIZE ? (file_size - offset) : CHUNK_SIZE);

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

        if (write_full(outfd, buf, got) != 0){
            free(buf);
            return -1;
        }

        offset += got;

        if (send_ack_cntn(cfd, offset) != 0){
            free(buf);
            return -1;
        }
    }

    fsync(outfd);
    free(buf);
    return 0;
}

// ===== LOOP por sesión: múltiples archivos hasta OP_END =====
void* handle_client(void *arg) {
    int cfd = (int)(intptr_t)arg;

    // Bloqueante
    int fl = fcntl(cfd, F_GETFL, 0);
    if (fl & O_NONBLOCK) fcntl(cfd, F_SETFL, fl & ~O_NONBLOCK);

    for (;;) {
        header_v2_t hdr;
        if (recv_header(cfd, &hdr) != 0) { close(cfd); return NULL; }

        WriteLog("[srv] header: name_len=%u, file_size=%llu, type=0x%02X\n",
               hdr.name_len, (unsigned long long)hdr.file_size, hdr.file_type);

        if (send_ack(cfd) != 0) { close(cfd); return NULL; }

        // Fin de lote 
        if (hdr.file_type == OP_END) {
            WriteLog("[srv] fin de lote (OP_END). elementos en cola: %zu\n", pqueue_count());
            // (opcional) pqueue_dump();
            size_t count = pqueue_count();

            for (size_t i = 0; i < count; i++)
            {
               CalcHist(pqueue_get_path(i));
               Clasificar(pqueue_get_path(i));
            }
            
            
            pqueue_clear();   // ← aquí se empieza un lote NUEVO
            continue;         // seguir leyendo más headers en la misma conexión
        }


        // Nombre
        char namebuf[513];
        if (recv_filename(cfd, namebuf, sizeof namebuf, hdr.name_len) != 0) {
            perror("[srv] recv filename");
            close(cfd); return NULL;
        }
        WriteLog("[srv] nombre: %s\n", namebuf);

        if (send_ack(cfd) != 0) { close(cfd); return NULL; }

        // Path salida
        const char *type_str = file_type_str(hdr.file_type);
        char path[512];
        if (prepare_output_path(path, sizeof path, namebuf, type_str) != 0) {
            close(cfd); return NULL;
        }

        // Crear y recibir
        int outfd = open_and_truncate(path, hdr.file_size);
        if (outfd < 0) { perror("[srv] open/truncate"); close(cfd); return NULL; }

        if (receive_file_data(cfd, outfd, hdr.file_size) != 0){
            perror("[srv] receive_file_data");
            close(outfd); close(cfd); return NULL;
        }
        close(outfd);

        WriteLog("[srv] archivo completo: %s (%llu bytes)\n",
               path, (unsigned long long)hdr.file_size);

        // Encolar metadatos en pqueue
        pqueue_entry_t e = {0};
        snprintf(e.name, sizeof e.name, "%s", namebuf);
        snprintf(e.path, sizeof e.path, "%s", path);
        e.size = hdr.file_size;
        e.type = hdr.file_type;

        if (pqueue_push(&e) != 0) {
            fprintf(stderr, "[srv] no se pudo encolar %s\n", namebuf);
        } else {
            WriteLog("[srv] encolado: %s (%llu B), pend=%zu\n",
                   e.name, (unsigned long long)e.size, pqueue_count());
                   pqueue_dump(); 
        }

        // vuelve al inicio y espera otro header
    }

    close(cfd);
    return NULL;
}
