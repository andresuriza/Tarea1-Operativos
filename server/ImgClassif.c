#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
  #define PATHSEP '\\'
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
  #define MKDIR(p) mkdir(p, 0755)
  #define PATHSEP '/'
#endif

static void die(const char* msg) { fprintf(stderr, "Error: %s\n", msg); exit(1); }

static const char* basename_from_path(const char* path) {
    const char* slash1 = strrchr(path, '/');
    const char* slash2 = strrchr(path, '\\');
    const char* base = path;
    if (slash1 && slash2) base = (slash1 > slash2) ? slash1 + 1 : slash2 + 1;
    else if (slash1)     base = slash1 + 1;
    else if (slash2)     base = slash2 + 1;
    return base;
}

static void ensure_dir(const char* path) {
    if (!path || !*path) return;
    // Create recursively: split by PATHSEP
    char tmp[1024];
    size_t n = strlen(path);
    if (n >= sizeof(tmp)) die("Path too long");
    strcpy(tmp, path);
    // strip trailing sep
    while (n > 0 && (tmp[n-1] == '/' || tmp[n-1] == '\\')) { tmp[n-1] = '\0'; n--; }
    if (n == 0) return;

    // Walk components
    char buf[1024] = {0};
    size_t len = 0;
    for (size_t i = 0; i < n; ++i) {
        char c = tmp[i];
        if (c == '/' || c == '\\') {
            if (len > 0) {
                if (MKDIR(buf) != 0) { /* ignore errors (exists is fine) */ }
            }
        } else {
            size_t bl = strlen(buf);
            if (bl + 2 >= sizeof(buf)) die("Path too long");
            buf[bl] = c; buf[bl+1] = '\0';
        }
        if (c == '/' || c == '\\') {
            size_t bl = strlen(buf);
            if (bl + 2 >= sizeof(buf)) die("Path too long");
            buf[bl] = PATHSEP; buf[bl+1] = '\0';
        }
    }
    if (MKDIR(buf) != 0) { /* ignore exists */ }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_image out_root_dir\n", argv[0]);
        return 1;
    }
    const char* in_path  = argv[1];
    const char* out_root = argv[2];

    int w, h, ch;
    // Load "as is" to preserve alpha if present; if channels are odd (2), we fallback to 3.
    unsigned char* img = stbi_load(in_path, &w, &h, &ch, 0);
    if (!img) die("Failed to load image (unsupported or not found).");

    if (!(ch == 1 || ch == 3 || ch == 4)) {
        // reload as 3 channels if e.g. 2 channels encountered
        stbi_image_free(img);
        img = stbi_load(in_path, &w, &h, &ch, 3);
        if (!img) die("Unsupported channel count.");
        ch = 3;
    }

    size_t pixels = (size_t)w * (size_t)h;
    // Compute sums; if grayscale (1ch), treat value as R=G=B=value
    unsigned long long sumR = 0, sumG = 0, sumB = 0;

    if (ch == 1) {
        for (size_t i = 0; i < pixels; ++i) {
            unsigned int v = img[i];
            sumR += v; sumG += v; sumB += v;
        }
    } else if (ch == 3 || ch == 4) {
        for (size_t i = 0, p = 0; i < pixels; ++i, p += ch) {
            sumR += img[p + 0];
            sumG += img[p + 1];
            sumB += img[p + 2];
        }
    }

    // Decide dominant color (tie-break: R > G > B)
    const char* folder = "red";
    if (sumG > sumR && sumG >= sumB) folder = "green";
    else if (sumB > sumR && sumB > sumG) folder = "blue";
    // else "red" stays

    // Build output folder and filename
    char out_dir[1024];
    snprintf(out_dir, sizeof(out_dir), "%s%c%s", out_root, PATHSEP, folder);
    ensure_dir(out_root);
    ensure_dir(out_dir);

    const char* base = basename_from_path(in_path);

    // Strip original extension and save as .png in target folder
    char stem[512];
    const char* dot = strrchr(base, '.');
    size_t stem_len = dot ? (size_t)(dot - base) : strlen(base);
    if (stem_len >= sizeof(stem)) stem_len = sizeof(stem) - 1;
    memcpy(stem, base, stem_len); stem[stem_len] = '\0';

    char out_path[1200];
    snprintf(out_path, sizeof(out_path), "%s%c%s.png", out_dir, PATHSEP, stem);

    // Write PNG (keep original channels if 1/3/4)
    int ok = stbi_write_png(out_path, w, h, ch, img, w * ch);
    stbi_image_free(img);
    if (!ok) die("Failed to write PNG.");

    printf("Dominant: %s\nSaved: %s\n", folder, out_path);
    return 0;
}
