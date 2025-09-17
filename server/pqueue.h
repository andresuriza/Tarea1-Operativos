// pqueue.h
#ifndef PQUEUE_H
#define PQUEUE_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    char     name[513];
    char     path[512];
    uint64_t size;
    uint8_t  type;
} pqueue_entry_t;

int    pqueue_init(void);
void   pqueue_destroy(void);
int    pqueue_push(const pqueue_entry_t *entry);
size_t pqueue_count(void);

// Nuevo: vaciar cola (conserva la capacidad)
void   pqueue_clear(void);

// (opcional si ya lo tienes)
void   pqueue_dump(void);

const char *pqueue_get_path(size_t pos);  // NULL si fuera de rango

#endif // PQUEUE_H
