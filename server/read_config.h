#ifndef READ_CONFIG_H
#define READ_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Lee el puerto desde un archivo de configuración.
 * Espera una línea con el formato: "Puerto:<numero>"
 *
 * @param ruta Ruta al archivo config (ej: "../config.conf")
 * @return El número de puerto leído, o -1 en caso de error
 */
int leer_puerto_desde_config(const char *ruta);

#ifdef __cplusplus
}
#endif

#endif // READ_CONFIG_H
