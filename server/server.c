#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

void* accept_loop(void* arg){
    int sfd = (int)(intptr_t)arg;
    while(1){
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);

        int cfd = accept(sfd, (struct sockaddr*)&cli, &len);
        if(cfd <0){
            if(errno == EINTR){
                continue;
            }
            perror("Accept fallido");
            break;
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        printf("Cliente conectado %s:%d\n", ip, ntohs(cli.sin_port));
        sleep(100); // simular trabajo
        close(cfd);

    }
    return NULL;
}


int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error creando socket");
        return 1;
    }
    printf("Socket creado exitosamente, fd=%d\n", server_fd);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error configurando opciones del socket");
        close(server_fd);
        return 1;
    }



    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // todas las interfaces
    addr.sin_port = htons(1717);

    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr))<0){
        perror("Bind falldo");
        close(server_fd);
        return 1;
    }

    if(listen(server_fd, SOMAXCONN)<0){
        perror("Listen fallido");
        close(server_fd);
        return 1;
    }
    printf("Servidor escuchando en puerto 1717...\n");
    pthread_t th;
    if(pthread_create(&th, NULL, accept_loop,(void*)(intptr_t)server_fd)!=0){
        perror("Error creando thread");
        close(server_fd);
        return 1;
    }
    printf("Hilo aceptador en ejecucion\n");

    while (1) { sleep(1); }

    return 0;
}