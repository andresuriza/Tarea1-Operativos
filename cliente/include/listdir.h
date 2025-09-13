// listdir.h
#ifndef LISTDIR_H
#define LISTDIR_H
#include <stddef.h>

// Devuelve un arreglo dinámico de strings (char*) con los nombres de imágenes.
// - out_count: escribe ahí cuántos nombres hay.
// - Retorno: NULL si no hay imágenes o hay error (en ambos casos *out_count = 0).
// Propiedad de memoria: el llamador debe liberar cada string y el arreglo.
char** list_dir_images(const char* path, size_t* out_count);

// (opcional) helper para liberar
void free_string_array(char** arr, size_t count);

#endif
