#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "listdir.h"
#include "file_metadata.h"
#include "send_data.h"
#include "ui_client.h"
#define MAX_NAME 256




int main() {

    const char *server_name = "host.docker.internal";
    struct hostent *he = gethostbyname(server_name);
    const int port = 1717;

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock <0){
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (!he || he->h_addrtype != AF_INET) {
        fprintf(stderr, "No pude resolver %s a IPv4\n", server_name);
        close(sock);
        return 1;
    }

    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        perror("Connection Failed");
        close(sock);
        return 1;
    }

    printf("[cliente] Conectado a %s:%d\n", server_name, port);
    while (1) {
        size_t count=0;
        char **names = show_image_menu("/data", &count);
        if (!names) { /* maneja sin terminar el proceso si querés */ break; }

        char filename[MAX_NAME];
        prompt_res_t r = prompt_and_validate_filename(filename, sizeof filename, names, count);
        if (r == PROMPT_EXIT) {
            free_string_array(names, count);
            // enviar EXIT aquí:
            if (send_exit(sock) != 0) { /* log error */ }
            break; // salir del while → luego close(sock)
        } else if (r == PROMPT_SELECTED) {
            // enviar archivo aquí:
            if (send_data(sock, filename) != 0) { /* log error y decidir si continuar */ }
            // seguir en el while para otra imagen
        } else if (r == PROMPT_AGAIN) {
            // no salir: volver a mostrar el menú
        } else { // PROMPT_ERROR
            // decide si romper el loop o volver a intentar
        }
        free_string_array(names, count);
    }

    close(sock);
    return 0;

}
