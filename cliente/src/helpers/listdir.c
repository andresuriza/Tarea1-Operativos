#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h> 
#include <stddef.h> 
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <stdlib.h>   // <-- agrega esto


static const char* get_ext(const char* name){
    const char* dot = strrchr(name, '.');
    if(!dot || dot == name){
        return NULL;
    }
    return dot;
}
static bool is_image_name(const char* name){
    const char* ext = get_ext(name);
    if(!ext){
        return false;
    }

    return
     
    strcasecmp(ext, ".jpg") == 0 || 
    strcasecmp(ext, ".jpeg") == 0 ||
    strcasecmp(ext, ".png") == 0 || 
    strcasecmp(ext, ".gif") == 0;
}

char** list_dir_images(const char* path, size_t* out_count){
    if (!out_count) return NULL;
    *out_count = 0;

    DIR* d = opendir(path);
    if (!d){
        perror("[cliente] opendir");
        return NULL;
    }

    char** names = NULL;
    size_t cap = 0, n = 0;

    struct dirent* ent;
    while ((ent = readdir(d))){
        if (ent->d_name[0] == '.') continue; // oculta . y ..
        // Confirmar archivo regular:
        char full[4096];
        snprintf(full, sizeof(full), "%s/%s", path, ent->d_name);

        struct stat st;
        if (stat(full, &st) == 0 && S_ISREG(st.st_mode) && is_image_name(ent->d_name)){
            // crecer el arreglo si hace falta
            if (n == cap){
                size_t newcap = cap ? cap*2 : 8;
                char** tmp = realloc(names, newcap * sizeof(char*));
                if (!tmp){
                    // error: liberar lo ya acumulado
                    for (size_t i=0;i<n;i++) free(names[i]);
                    free(names);
                    closedir(d);
                    return NULL;
                }
                names = tmp;
                cap = newcap;
            }
            names[n] = strdup(ent->d_name); // copia el nombre
            if (!names[n]){
                for (size_t i=0;i<n;i++) free(names[i]);
                free(names);
                closedir(d);
                return NULL;
            }
            n++;
        }
    }
    closedir(d);

    if (n == 0){
        // sin imágenes válidas
        free(names);
        return NULL; // *out_count ya es 0
    }

    // (opcional) encoger exacto
    char** shrink = realloc(names, n * sizeof(char*));
    if (shrink) names = shrink;

    *out_count = n;
    return names;
}

void free_string_array(char** arr, size_t count){
    if (!arr) return;
    for (size_t i=0;i<count;i++) free(arr[i]);
    free(arr);
}