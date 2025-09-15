// ui_client.h
#ifndef UI_CLIENT_H
#define UI_CLIENT_H
#include <stddef.h>

typedef enum {
    PROMPT_SELECTED = 1,
    PROMPT_EXIT     = 0,
    PROMPT_AGAIN    = -1,
    PROMPT_ERROR    = -2
} prompt_res_t;



prompt_res_t prompt_and_validate_filename(char *out, size_t cap, char **names, size_t count);

char **show_image_menu(const char *path, size_t *count);

#endif
