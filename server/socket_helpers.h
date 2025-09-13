#ifndef SOCKET_HELPERS_H
#define SOCKET_HELPERS_H

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h> // Para size_t

/**
 * @brief Lee exactamente 'n' bytes del descriptor de archivo 'fd'.
 *
 * Esta función es un wrapper para `recv` que garantiza que se reciban todos
 * los bytes solicitados, manejando interrupciones (`EINTR`).
 *
 * @param fd El descriptor de archivo del socket.
 * @param buf El buffer para almacenar los datos recibidos.
 * @param n El número de bytes a recibir.
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int recv_exact(int fd, void *buf, size_t n);

/**
 * @brief Envía exactamente 'n' bytes a través del descriptor de archivo 'fd'.
 *
 * Esta función es un wrapper para `send` que asegura que todos los bytes
 * sean enviados, manejando interrupciones (`EINTR`).
 *
 * @param fd El descriptor de archivo del socket.
 * @param buf El buffer que contiene los datos a enviar.
 * @param n El número de bytes a enviar.
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
int send_all(int fd, const void *buf, size_t n);

#endif // SOCKET_HELPERS_H