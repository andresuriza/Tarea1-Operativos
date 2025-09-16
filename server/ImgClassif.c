#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MKDIR(p) mkdir(p, 0755)
#define PATHSEP '/'

// Funcion para imprimir errores
static void p_error(const char* msg) 
{ 
    fprintf(stderr, "Error: %s\n", msg); exit(1); 
}

// Obtiene nombre y extension de imagen
static const char* GetFileN(const char* path) 
{
    const char* slash = strrchr(path, '/');
    const char* base = path;

    base = slash + 1;

    return base;
}

// Verifica directorio
static void CheckDir(const char* path) 
{
    if (!path || !*path) 
        return;

    char tmp[1024];
    size_t n = strlen(path);

    if (n >= sizeof(tmp)) 
        p_error("Path too long");

    strcpy(tmp, path);

    while (n > 0 && (tmp[n-1] == '/')) 
    { 
        tmp[n-1] = '\0'; n--; 
    }

    if (n == 0) 
        return;

    char buf[1024] = {0};
    size_t len = 0;
    for (size_t i = 0; i < n; ++i) 
    {
        char c = tmp[i];
        if (c == '/') 
        {
            if (len > 0) 
            {
                if (MKDIR(buf) != 0) { /* ignora errores */ }
            }
        } 
        
        else 
        {
            size_t bl = strlen(buf);
            if (bl + 2 >= sizeof(buf)) p_error("Path muy largo");
            buf[bl] = c; buf[bl+1] = '\0';
        }

        if (c == '/') 
        {
            size_t bl = strlen(buf);
            if (bl + 2 >= sizeof(buf)) p_error("Path muy largo");
            buf[bl] = PATHSEP; buf[bl+1] = '\0';
        }
    }

    if (MKDIR(buf) != 0) { /* ignorar si existe */ }
}

int main() {
    const char* img_in  = "../imagenes_in/ocean.jpg";
    const char* img_out = "../imagenes_out";
    
    int w, h, ch;

    unsigned char* img = stbi_load(img_in, &w, &h, &ch, 0);

    if (!img) 
        p_error("Error al cargar imagen");

    if (!(ch == 1 || ch == 3 || ch == 4)) 
    {
        stbi_image_free(img);
        img = stbi_load(img_in, &w, &h, &ch, 3);
        
        if (!img) 
            p_error("Cantidad de canales de imagen invalido");
        
        ch = 3;
    }

    size_t pixeles = (size_t)w * (size_t)h;

    unsigned long long sumR = 0, sumV = 0, sumA = 0;

    if (ch == 1) 
    {
        for (size_t i = 0; i < pixeles; ++i) 
        {
            unsigned int val = img[i];
            sumR += val; 
            sumV += val; 
            sumA += val;
        }
    } 
    
    else if (ch == 3 || ch == 4) 
    {
        for (size_t i = 0, p = 0; i < pixeles; ++i, p += ch) 
        {
            sumR += img[p + 0];
            sumV += img[p + 1];
            sumA += img[p + 2];
        }
    }

    const char* folder = "Rojo";

    if (sumV > sumR && sumV >= sumA) 
        folder = "Verde";

    else if (sumA > sumR && sumA > sumV) 
        folder = "Azul";

    char folderOut[1024];
    snprintf(folderOut, sizeof(folderOut), "%s%c%s", img_out, PATHSEP, folder);
    CheckDir(img_out);
    CheckDir(folderOut);

    const char* base = GetFileN(img_in);

    char out_path[1200];
    snprintf(out_path, sizeof(out_path), "%s%c%s", folderOut, PATHSEP, base);
    
    int ok = stbi_write_png(out_path, w, h, ch, img, w * ch);
    stbi_image_free(img);
    
    if (!ok) 
        p_error("Error al escribir PNG.");
    
    return 0;
}
