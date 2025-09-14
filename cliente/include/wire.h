#pragma once
#include <stdint.h>
#include <stddef.h>

#define ACK_CODE_OK 0xAA
#define ACK_CODE_CNTN 0x55

#pragma pack(push, 1)  // ← fuerza alineación 1 byte
typedef struct {
    uint8_t  code;
    uint64_t next_offset;
} ack_t;
#pragma pack(pop)      // ← restaura alineación

// Ya tienes htonll/ntohll; decláralos si están en otro header
uint64_t htonll_u64(uint64_t x);
uint64_t ntohll_u64(uint64_t x);

// API de conveniencia usando tus send_all/recv_exact
int send_u64(int sock, uint64_t v);
int recv_u64(int sock, uint64_t *out);


