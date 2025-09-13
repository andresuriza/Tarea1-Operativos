#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <stdint.h>
#include <stddef.h>   // para size_t

// Incluye tu enum de tipos de archivo
#include "FileType.h"

// Convierte un uint64_t de host a network byte order (big endian)
uint64_t htonll(uint64_t v);

// Devuelve el FileType según la extensión (.jpg/.jpeg/.png/.gif)
// Retorna -1 si no se reconoce.
int file_type_from_name(const char *name);

// Construye un header de 11 bytes con:
// [0..1]   uint16 name_len
// [2..9]   uint64 file_size
// [10]     uint8  file_type
// Retorna siempre 11 (tamaño del header).
size_t build_header_v2(uint8_t out[11], const char *filename);

#endif // FILE_METADATA_H
