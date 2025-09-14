#include <stdint.h>
#include <string.h>
#include "FileType.h"
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "wire.h"


static int file_type_from_name(const char *name){
    const char *dot = strrchr(name, '.');
    if (!dot || dot[1]=='\0') return -1;
    if (!strcasecmp(dot, ".jpg") || !strcasecmp(dot, ".jpeg")) return FILETYPE_JPG;
    if (!strcasecmp(dot, ".png")) return FILETYPE_PNG;
    if (!strcasecmp(dot, ".gif")) return FILETYPE_GIF;
    return -1;
}
size_t build_header_v2(uint8_t out[11], const char *filename) {
    // ruta completa
    char fullpath[512];
    snprintf(fullpath, sizeof fullpath, "/data/%s", filename);

    // longitud del nombre
    uint16_t name_len = (uint16_t)strlen(filename);

    // tamaño del archivo
    struct stat st;
    stat(fullpath, &st);
    uint64_t file_size = (uint64_t)st.st_size;

    // tipo
    uint8_t file_type = (uint8_t)file_type_from_name(filename);

    // serializar
    uint16_t n_be = htons(name_len);
    uint64_t sz_be = htonll_u64(file_size);
    memcpy(out+0, &n_be, 2);
    memcpy(out+2, &sz_be, 8);
    out[10] = file_type;

    return 11;
}

