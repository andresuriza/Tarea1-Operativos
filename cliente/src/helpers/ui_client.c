#include <stdio.h>
#include <stddef.h>   
#include <string.h>   
#include "listdir.h" 
#include "ui_client.h"
#include <strings.h> // para strcasecmp()

prompt_res_t prompt_and_validate_filename(char *out, size_t cap,
                                          char **names, size_t count) {
    printf("Ingrese el nombre del archivo a procesar (o EXIT para terminar): ");
    
    if (fgets(out, cap, stdin) == NULL) {
        printf("Error leyendo la entrada\n");
        return PROMPT_ERROR;
    }

    // Eliminar el salto de línea final
    out[strcspn(out, "\n")] = '\0';

    // Verificar si es "EXIT" (ignorar mayúsculas/minúsculas)
    if (strcasecmp(out, "EXIT") == 0) {
        return PROMPT_EXIT;
    }

    // Validar si está en la lista de nombres
    for (size_t i = 0; i < count; i++) {
        if (strcmp(out, names[i]) == 0) {
            return PROMPT_SELECTED;
        }
    }

    printf("[cliente] El archivo '%s' no está en la lista. Intente de nuevo.\n", out);
    return PROMPT_AGAIN;
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