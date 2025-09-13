#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

/*
 * @brief Lee exactamente n bytes del descriptor de archivo fd.
 * @param fd El descriptor de archivo del socket.
 * @param buf El buffer donde se almacenarán los datos recibidos.
 * @param n El número de bytes a leer.
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
static int recv_exact(int fd, void *buf, size_t n);

/*
 * @brief Envía exactamente n bytes a través del descriptor de archivo fd.
 * @param fd El descriptor de archivo del socket.
 * @param buf El buffer con los datos a enviar.
 * @param n El número de bytes a enviar.
 * @return 0 si la operación fue exitosa, -1 en caso de error.
 */
static int send_all(int fd, const void *buf, size_t n);

/*
 * @brief Maneja la comunicación con un cliente en un thread separado.
 * @param arg Un puntero al descriptor de archivo del socket del cliente.
 * @return Siempre NULL.
 */
void* handle_client(void *arg);

#endif // HANDLE_CLIENT_H