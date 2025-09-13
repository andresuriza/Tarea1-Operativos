#include <stdio.h>
#include <stddef.h>   
#include <string.h>   
#include "listdir.h" 
#include "ui_client.h"


int prompt_and_validate_filename(char *out, size_t cap,
                                 char **names, size_t count) {
    printf("Ingrese el nombre del archivo a procesar: ");
    if (fgets(out, cap, stdin) == NULL) {
        printf("Error leyendo el nombre del archivo\n");
        return 0;
    }
    out[strcspn(out, "\n")] = '\0'; // quitar '\n'

    // Validar que esté en la lista
    for (size_t i = 0; i < count; i++) {
        if (strcmp(out, names[i]) == 0) {
            return 1; // encontrado y válido
        }
    }

    printf("[cliente] El archivo '%s' no está en la lista\n", out);
    return 0;
}

char **show_image_menu(const char *path, size_t *count) {
    char **names = list_dir_images(path, count);
    if (!names || *count == 0) {
        printf("[cliente] No hay imágenes válidas en %s\n", path);
        return NULL;
    }

    printf("[cliente] Archivos disponibles:\n");
    for (size_t i = 0; i < *count; i++) {
        printf("  - %s\n", names[i]);
    }

    return names;
}