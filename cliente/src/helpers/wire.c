#include <stdint.h>
#include <string.h>
#include "send_data.h"

uint64_t htonll_u64(uint64_t v) {
    uint32_t hi = (uint32_t)(v >> 32);
    uint32_t lo = (uint32_t)(v & 0xFFFFFFFFu);
    hi = htonl(hi); lo = htonl(lo);
    return ((uint64_t)lo << 32) | hi;
}
uint64_t ntohll_u64(uint64_t v) {
    return htonll_u64(v); // es simétrico
}

int send_u64(int sock, uint64_t v) {
    uint64_t be = htonll_u64(v);
    return send_all(sock, &be, sizeof be);
}
int recv_u64(int sock, uint64_t *out){
    uint64_t be;
    if (recv_exact(sock, &be, sizeof be) != 0) return -1;
    *out = htonll_u64(be);
    return 0;
}