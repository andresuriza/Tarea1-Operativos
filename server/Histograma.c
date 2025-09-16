#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

void histogram(unsigned char image_in[900][1600],unsigned char image_out[900][1600], 
    unsigned long hist[256],unsigned long eHist[256],float cdf[256]) {
    #define lines 900
    #define columns 1600

    int i,j;

    int pixels = lines*columns;

    // original histogram

    for (i = 0; i < 256; i++) {
        hist[i]=0;
    }

    for (i = 0; i < lines; i++) {
        for (j = 0; j < columns; j++) {
              hist[image_in[i][j]]++;
        }
    }

    // Cumulative Distribution Function
    float cdfmax=256, cdfmin=1;

    for (i = 0; i < 256; i++) {
        cdf[i] = 0;
        for (j = 0; j <= i; j++) {
            cdf[i] += hist[j];
        }
    }

    // Equalized Histogram

    for (i = 0; i < 256; i++) {
        eHist[i] = ((cdf[i]-cdfmin)/((lines*columns)-cdfmin))*255;
    }

    // Final Image

    for (i = 0; i < lines; i++) {
        for (j = 0; j < columns; j++) {
            image_out[i][j] = cdf[image_in[i][j]]*255;
        }
    }
}   


int main()
{
    /*
    FILE *fIn = fopen("silksong_bw.bmp", "rb");
    FILE *fIn2 = fopen("silksong_bw.bmp", "rb");
    FILE *fOut = fopen("silke.bmp", "wb");

    if (!fIn || !fOut)
    {
        printf("File error.\n");
        return 0;
    }

    // Lectura y escritura de headers BMP
    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, fIn);
    fwrite(header, sizeof(unsigned char), 54, fOut);

    int width = *(int*)&header[18];
    int height = abs(*(int*)&header[22]);
    int stride = (width * 3 + 3) & ~3;
    int padding = stride - width * 3;

    
    //Filtro blanco y negro
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            fread(pixel, 3, 1, fIn);
            unsigned char gray = pixel[0] * 0.3 + pixel[1] * 0.58 + pixel[2] * 0.11;
            memset(pixel, gray, sizeof(pixel));
            fwrite(&pixel, 3, 1, fOut);
        }
            fread(pixel, padding, 1, fIn);
            fwrite(pixel, padding, 1, fOut);
    }
    

    fclose(fOut);
    fclose(fIn);
    */

    FILE *fp = fopen("boat.png", "rb");
    FILE *fp2 = fopen("boat2.jpg", "wb");
    int i,j;

    unsigned char image_in[372][299];
    unsigned char image_out[372][299];
    unsigned long hist[256];
    unsigned long eHist[256];
    float cdf[256];
    
    char buf[BUFSIZ];
    size_t bytes_read;
    
    /*
    for (int i=0; i < 900; i++) 
    {
        for (int j=0; j<1600; j++) 
            image_in[i][j] = getc(fp);
    }
            */

    for (int i=0; i < 372; i++) 
    {
        for (int j=0; j<299; j++) 
            image_in[i][j] = getc(fp);
    }

    for (int i=0; i < 372; i++) 
    {
        for (int j=0; j<299; j++) 
            fwrite(image_in[i][j], 1, 1, fp2);
    }

    //histogram(image_in, image_out, hist, eHist, cdf);

    /*
    while((bytes_read = fread(buf, 1, sizeof(buf), fp))) 
    {
        // Write the hunk, but only as much as was read.
        fwrite(buf, 1, bytes_read, fp2);
    }
    */

    fclose(fp);
    fclose(fp2);

    return 0;
}