#include <stdio.h>
#include <unistd.h>
#include "ImgFunciones.c"

const char *filename = "/etc/server/config.conf";

int GetPort()
{
    int port;
    FILE *file = fopen(filename, "r");

    if (fscanf(file, "Puerto:%d%*c", &port) != 1) 
            printf("Error al leer configuracion\n");
    
    fclose(file);

    return port;
}

char* Get_Dircolores()
{
    int port;
    char* pathColores = malloc(50);


    FILE *file = fopen(filename, "r");

    if (fscanf(file, "Puerto:%d%*c", &port) != 1) 
            printf("Error al leer configuracion\n");

    if (fscanf(file, "DirColores:%s%*c", pathColores) != 1) 
            printf("Error al leer configuracion\n");

    fclose(file);

    return pathColores;
}

char* Get_Dirhisto()
{
    int port;
    char* pathColores = malloc(50);
    char* pathHisto = malloc(50);

    FILE *file = fopen(filename, "r");

    if (fscanf(file, "Puerto:%d%*c", &port) != 1) 
            printf("Error al leer configuracion\n");

    if (fscanf(file, "DirColores:%s%*c", pathColores) != 1) 
            printf("Error al leer configuracion\n");

    if (fscanf(file, "DirHisto:%s%*c", pathHisto) != 1) 
            printf("Error al leer configuracion\n");

    fclose(file);

    return pathHisto;
}

// Funcion para escribir en un log
void WriteLog(const char* msg) 
{
    int port;
    char* pathColores = malloc(50);
    char* pathHisto = malloc(50);
    char* pathLog = malloc(50);

    FILE *file = fopen(filename, "r");

    if (fscanf(file, "Puerto:%d%*c", &port) != 1) 
            printf("Error al leer configuracion\n");

    if (fscanf(file, "DirColores:%s%*c", pathColores) != 1) 
            printf("Error al leer configuracion\n");

    if (fscanf(file, "DirHisto:%s%*c", pathHisto) != 1) 
            printf("Error al leer configuracion\n");

    if (fscanf(file, "DirLog:%s%*c", pathLog) != 1) 
            printf("Error al leer configuracion\n");
       
    fclose(file);

    FILE *log = fopen(pathLog, "a");

    fprintf(log, "%s\n", msg);

    fclose(log);
}