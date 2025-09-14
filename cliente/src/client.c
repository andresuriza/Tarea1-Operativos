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

    size_t count = 0;

    char **names = show_image_menu("/data", &count);

    if (!names) {
        printf("No se encontraron imágenes\n");
        close(sock);
        return 1;
    }

    char filename[MAX_NAME];
    if (!prompt_and_validate_filename(filename, sizeof filename, names, count)) {
        free_string_array(names, count);
        close(sock);
        return 1;
    }

    if (send_data(sock, filename) != 0) {
        perror("[cliente] fallo al enviar header/nombre");
        free_string_array(names, count);
        close(sock);
        return 1;
    }

    free_string_array(names, count);

    close(sock);
    return 0;

}
