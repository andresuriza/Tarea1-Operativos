#include <stdio.h>
#include <unistd.h>
#include "ImgFunciones.c"

int port;
char imgsOut[50];
char imgsIn[50];
char logPath[50];

// Lee archivo de configuracion y guarda sus valores en variables
void GetConfig() 
{
    const char *filename = "/etc/server/config.conf";
    FILE *file = fopen(filename, "r");

    if (file == NULL) 
        printf("No se encontro archivo \n");

    else 
    {
        if (fscanf(file, "Puerto:%d%*c", &port) != 1) 
            printf("Failed to read data from file.\n");
        
        if (fscanf(file, "DirColores:%s%*c", imgsOut) != 1) 
            printf("Failed to read data from file.\n");
        
        if (fscanf(file, "DirHisto:%s%*c", imgsIn) != 1) 
            printf("Failed to read data from file.\n");

        if (fscanf(file, "DirLog:%s%*c", logPath) != 1) 
            printf("Failed to read data from file.\n");

        fclose(file);
    }
}

/* Funcion para escribir en un log
*  Nota: Directorio esta actualmente definido para mi computadora (Andres)
*  Solo escribe texto de prueba para probar daemon
*/
void WriteLog() 
{
    const char *filename = "/home/andres/Documents/tarea-operativos/log/logs.txt";
    FILE *file = fopen(filename, "a");

    fprintf(file, "Un daemon escribiendo \n");

    fclose(file);
}

int main() {
    GetConfig(imgsIn);

    CalcHist(imgsIn);
    Clasificar(imgsIn);

    return 0;
}