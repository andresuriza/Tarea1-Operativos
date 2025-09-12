#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>   // gethostbyname

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

        close(sock);
    return 0;

}
