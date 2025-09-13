#ifndef SEND_DATA_H
#define SEND_DATA_H

#include <stddef.h>   // size_t
#include <stdint.h>   // uint8_t, uint16_t

// Envía exactamente 'len' bytes por el socket 'fd'.
// Devuelve 0 si OK, -1 si error.
int send_all(int fd, const void *buf, size_t len);

// Imprime un buffer en hexadecimal (debug).
void dump_hex(const unsigned char *p, size_t n);

// Construye el header v2 y envía header + nombre del archivo.
// Devuelve 0 si OK, -1 si error.
int send_header_and_name(int sock, const char *filename);

#endif // SEND_UTILS_H
