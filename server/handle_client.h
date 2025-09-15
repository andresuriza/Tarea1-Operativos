// handle_client.h (agrega/ajusta estas líneas)

#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t name_len;
    uint64_t file_size;
    uint8_t  file_type;
} header_v2_t;

#define ACK_CODE_CNTN 0x55
#define CHUNK_SIZE    (64*1024)  // Debe coincidir con el cliente

// Estructura de ACK en el wire
#pragma pack(push,1)
typedef struct {
    uint8_t  code;        // 0x55 = continuar
    uint64_t next_offset; // big-endian
} ack_t;
#pragma pack(pop)

// Aux
int recv_header(int cfd, header_v2_t *out);
int recv_filename(int cfd, char *out, size_t maxlen, uint16_t name_len);
const char* file_type_str(uint8_t file_type);
int prepare_output_path(char *out_path, size_t cap, const char *namebuf, const char *type_str);
int open_and_truncate(const char *path, uint64_t file_size);
int send_ack(int cfd);

// NUEVO: ACK de 'continue' y recepción de archivo
int send_ack_cntn(int cfd, uint64_t next_off_host);
int receive_file_data(int cfd, int outfd, uint64_t file_size);

void* handle_client(void *arg);

#ifdef __cplusplus
}
#endif
