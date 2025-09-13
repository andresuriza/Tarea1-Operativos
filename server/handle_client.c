#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include "handle_client.h"
#include "socket_helpers.h"

void* handle_client(void *arg){
    int cfd = (int)(intptr_t)arg;

    // Asegurar modo bloqueante (por si alguien puso O_NONBLOCK en otro lado)
    int fl = fcntl(cfd, F_GETFL, 0);
    if (fl & O_NONBLOCK) fcntl(cfd, F_SETFL, fl & ~O_NONBLOCK);

    // 1) Esperar bloqueante el header (11 bytes)
    uint8_t hdr[11];
    if (recv_exact(cfd, hdr, sizeof hdr) != 0) { close(cfd); return NULL; }
    printf("[srv] header recibido (11B)\n");

    // 2) Enviar ACK del header
    uint8_t ack = 0xAA;
    if (send_all(cfd, &ack, 1) != 0) { close(cfd); return NULL; }
    printf("[srv] envié ACK header (0xAA)\n");

    uint16_t name_len = ((u_int16_t)hdr[0] << 8) | (u_int16_t)hdr[1];
    if(name_len ==0 || name_len >512){
        perror("[srv] Name length out of range");
        close(cfd);
        return NULL;
    }
    char namebuf[513];
    if(recv_exact(cfd, namebuf,name_len)!=0){
        perror("[srv] recv_exact filename");
        close(cfd);
        return NULL;
    }
    namebuf[name_len] = '\0';
    printf("[srv] nombre recibido: '%s'\n", namebuf);

    uint8_t ack2 = 0xAA;
    if (send_all(cfd, &ack2, 1) != 0) {
        perror("[srv] send ack filename");
        close(cfd);
        return NULL;
    }
    printf("[srv] ACK por nombre enviado\n");

    return NULL;
}