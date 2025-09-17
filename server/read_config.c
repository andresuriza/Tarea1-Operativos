#include <stdio.h>

int leer_puerto_desde_config(const char *ruta_config) {
    FILE *f = fopen(ruta_config, "r");
    if (!f) {
        perror("fopen config");
        return -1;
    }

    char linea[256];
    int puerto = -1;

    while (fgets(linea, sizeof linea, f)) {
        if (strncmp(linea, "Puerto:", 7) == 0) {
            puerto = atoi(linea + 7);
            break;
        }
    }

    fclose(f);
    return puerto;
}