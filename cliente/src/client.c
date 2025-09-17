#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>

#include "listdir.h"
#include "file_metadata.h"
#include "send_data.h"
#include "ui_client.h"
#include "read_config.h"

#define MAX_NAME 256

static volatile sig_atomic_t g_stop = 0; // ctrl+c presionado
static int g_sock = -1;               // para que el handler lo cierre

static void on_sigint(int sig) {
    (void)sig;
    if (g_sock >= 0) { close(g_sock); g_sock = -1; }
    // devolver la acción por defecto y re-disparar SIGINT para terminar
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}





int main() {

    const char *server_name = "host.docker.internal";
    struct hostent *he = gethostbyname(server_name);
    int port = leer_puerto_desde_config("config.conf");
    if (port <= 0) {
        fprintf(stderr, "Puerto inválido\n");
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);


    int sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock <0){
        perror("Socket creation failed");
        return 1;
    }
    g_sock = sock;                 // el handler sabrá qué cerrar
    signal(SIGINT, on_sigint); 

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
        switch (r) {
        case PROMPT_EXIT:
            if (send_exit(sock) != 0) perror("[cliente] EXIT falló");
            // NO break del while → inicia otro lote si quieres
            printf("[cliente] Lote enviado");
            break;
        case PROMPT_SELECTED:
            if (send_data(sock, filename) != 0) perror("[cliente] envío falló");
            break;
        case PROMPT_AGAIN:
            break;
        case PROMPT_ERROR:
            fprintf(stderr, "[cliente] error leyendo entrada\n");
            break;
        case PROMPT_CLOSE:
            printf("[cliente] Cerrando conexión\n");
            close(sock);
            free_string_array(names, count);
            break;
        }
        free_string_array(names, count);
    }

    close(sock);
    return 0;

}
