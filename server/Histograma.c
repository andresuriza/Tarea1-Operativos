#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Funcion para imprimir errores
static void p_error(const char* msg) 
{ 
    fprintf(stderr, "Error: %s\n", msg); exit(1); 
}

// Crea histograma de ecualizacion en base a algoritmo
static void CrearHist(const uint8_t* data, size_t n, uint8_t map[256]) 
{
    size_t hist[256] = { 0 };

    for (size_t i=0; i<n; ++i) 
        hist[data[i]]++;

    size_t cdf = 0, cdf_min = 0, total = n;
    int found_min = 0;

    for (int i = 0; i < 256; ++i) 
    {
        cdf += hist[i];
        if (!found_min && hist[i] != 0) 
        { 
            cdf_min = cdf; found_min = 1; 
        }
    }

    if (!found_min || total == 0) 
    {
        for (int i = 0; i < 256; ++i) map[i] = (uint8_t)i;
        return;
    }

    cdf = 0;

    for (int i = 0; i < 256; ++i) 
    {
        cdf += hist[i];
        if (cdf <= cdf_min) 
        { 
            map[i] = 0; 
        }

        else 
        {
            double num = (double)(cdf - cdf_min);
            double den = (double)(total - cdf_min);
            if (den <= 0.0) 
            { 
                map[i] = (uint8_t)i; 
            }

            else 
            {
                int v = (int)(num * 255.0 / den + 0.5);
                if (v < 0) 
                    v = 0; 
                if (v > 255) 
                    v = 255;
                map[i] = (uint8_t)v;
            }
        }
    }
}

// Convierte de rgb a ycbcr
static inline void rgb_to_ycbcr(uint8_t R, uint8_t G, uint8_t B, uint8_t* Y, uint8_t* Cb, uint8_t* Cr) 
{
    double y  =  0.299*R + 0.587*G + 0.114*B;
    double cb = -0.168736*R - 0.331264*G + 0.5*B + 128.0;
    double cr =  0.5*R - 0.418688*G - 0.081312*B + 128.0;

    int iy = (int)(y + 0.5), icb = (int)(cb + 0.5), icr = (int)(cr + 0.5);

    if (iy < 0) 
        iy = 0; 

    if (iy > 255) 
        iy = 255;

    if (icb < 0) 
        icb = 0; 

    if (icb > 255) 
        icb = 255;

    if (icr < 0) 
        icr = 0; 

    if (icr > 255) 
        icr = 255;

    *Y = (uint8_t)iy; *Cb = (uint8_t)icb; *Cr = (uint8_t)icr;
}

// Convierte de ycbcr a rgb
static inline void ycbcr_to_rgb(uint8_t Y, uint8_t Cb, uint8_t Cr, uint8_t* R, uint8_t* G, uint8_t* B) 
{
    double y = (double)Y;
    double cb = (double)Cb - 128.0;
    double cr = (double)Cr - 128.0;

    double r = y + 1.402 * cr;
    double g = y - 0.344136 * cb - 0.714136 * cr;
    double b = y + 1.772 * cb;

    int ir = (int)(r + 0.5), ig = (int)(g + 0.5), ib = (int)(b + 0.5);

    if (ir < 0) 
        ir = 0; 

    if (ir > 255) 
        ir = 255;
        
    if (ig < 0) 
        ig = 0; 

    if (ig > 255) 
        ig = 255;

    if (ib < 0) 
        ib = 0; 

    if (ib > 255) 
        ib = 255;

    *R = (uint8_t)ir; 
    *G = (uint8_t)ig; 
    *B = (uint8_t)ib;
}

// Crea imagen ecualizada en base a los histogramas de la imagen
static void Ecualizador(uint8_t* img, int w, int h, int canales, int mode_rgb) 
{
    size_t pixels = (size_t) w * (size_t) h;

    // Si es blanco y negro
    if (canales == 1) 
    {
        uint8_t map[256];
        CrearHist(img, pixels, map);
        for (size_t i = 0; i < pixels; ++i) 
            img[i] = map[img[i]];

        return;
    }

    // Si es RGB
    if (mode_rgb) 
    {
        uint8_t *r = (uint8_t*)malloc(pixels), *g = (uint8_t*)malloc(pixels), *b = (uint8_t*)malloc(pixels);

        if (!r || !g || !b) 
            p_error("OOM");

        for (size_t i = 0, p = 0; i < pixels; ++i, p += canales) 
        {
            r[i] = img[p+0];
            g[i] = img[p+1];
            b[i] = img[p+2];
        }

        uint8_t mr[256], mg[256], mb[256];
        CrearHist(r, pixels, mr);
        CrearHist(g, pixels, mg);
        CrearHist(b, pixels, mb);

        for (size_t i = 0, p = 0; i < pixels; ++i, p += canales) 
        {
            img[p+0] = mr[ img[p+0] ];
            img[p+1] = mg[ img[p+1] ];
            img[p+2] = mb[ img[p+2] ];
        }

        free(r); 
        free(g); 
        free(b);
    } 
    
    // Si es YCbCr
    else 
    {
        uint8_t *Y = (uint8_t*)malloc(pixels);
        uint8_t *Cb = (uint8_t*)malloc(pixels);
        uint8_t *Cr = (uint8_t*)malloc(pixels);

        if (!Y || !Cb || !Cr) 
            p_error("OOM");

        for (size_t i = 0, p = 0; i < pixels; ++i, p += canales) 
        {
            uint8_t R = img[p+0], G = img[p+1], B = img[p+2];
            rgb_to_ycbcr(R, G, B, &Y[i], &Cb[i], &Cr[i]);
        }

        uint8_t mapY[256];
        CrearHist(Y, pixels, mapY);
        for (size_t i = 0; i < pixels; ++i) 
            Y[i] = mapY[Y[i]];

        for (size_t i = 0, p = 0; i < pixels; ++i, p += canales) 
        {
            uint8_t R, G, B;
            ycbcr_to_rgb(Y[i], Cb[i], Cr[i], &R, &G, &B);
            img[p+0] = R; img[p+1] = G; img[p+2] = B;
        }

        free(Y); 
        free(Cb); 
        free(Cr);
    }
}

// Escribe imagen en base a su extension
static int WriteFile(const char* path, int w, int h, int nchan, const uint8_t* data) {
    const char* dot = strrchr(path, '.');
    const char* ext = dot ? dot+1 : "";
    int ok = 0;

    if (!ext[0] || strcasecmp(ext, "png") == 0)
        ok = stbi_write_png(path, w, h, nchan, data, w * nchan);

    else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
        ok = stbi_write_jpg(path, w, h, nchan, data, 95);

    else if (strcasecmp(ext, "bmp") == 0)
        ok = stbi_write_bmp(path, w, h, nchan, data);

    else if (strcasecmp(ext, "tga") == 0)
        ok = stbi_write_tga(path, w, h, nchan, data);

    else
        ok = stbi_write_png(path, w, h, nchan, data, w * nchan);
        
    return ok;
}

int main() {
    const char* in_path = "../imagenes_in/labrador_bw.jpg";
    const char* out_path = "out.jpg";

    // Cambiar a 1 si es YCbCr, (solo luminancia)
    int mode_rgb = 0; 

    int w, h, ch;
    uint8_t* img = stbi_load(in_path, &w, &h, &ch, 0);
    
    if (!img) 
        p_error("Error al cargar imagen");

    if (!(ch == 1 || ch == 3 || ch == 4)) 
    {
        uint8_t* convertido = stbi_load(in_path, &w, &h, &ch, 3);

        if (!convertido)
            stbi_image_free(img); p_error("Cantidad invalida de canales"); 
        
        stbi_image_free(img);
        img = convertido; 
        ch = 3;
    }

    Ecualizador(img, w, h, ch, mode_rgb);

    if (!WriteFile(out_path, w, h, ch, img)) 
    {
        stbi_image_free(img);
        p_error("Error al escribir imagen");
    }

    stbi_image_free(img);
    return 0;
}
