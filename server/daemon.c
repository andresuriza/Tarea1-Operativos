#include <stdio.h>
#include <unistd.h>

int port;
char imgsOut[50];
char imgsIn[50];
char logPath[50];

void GetConfig() {
    const char *filename = "/etc/server/config.conf";
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("No se encontro archivo \n");
    }
    else {
        if (fscanf(file, "Puerto:%d%*c", &port) != 1) {
            printf("Failed to read data from file.\n");
        }

        if (fscanf(file, "DirColores:%s%*c", imgsOut) != 1) {
            printf("Failed to read data from file.\n");
        }

        if (fscanf(file, "DirHisto:%s%*c", imgsIn) != 1) {
            printf("Failed to read data from file.\n");
        }

        if (fscanf(file, "DirLog:%s%*c", logPath) != 1) {
            printf("Failed to read data from file.\n");
        }

        fclose(file);
    }
}

void WriteLog() {
    const char *filename = "/home/andres/Documents/tarea-operativos/log/logs.txt";
    FILE *file = fopen(filename, "a");

    fprintf(file, "Un daemon escribiendo \n");

    fclose(file);
}

int main() {
    GetConfig();
    
    while(1) {
        WriteLog();
        sleep(5);
    }

    return 0;
}