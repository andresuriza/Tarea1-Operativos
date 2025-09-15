#ifndef PQUEUE_H
#define PQUEUE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    char     name[513];   // nombre del archivo
    char     path[512];   // ruta en disco
    uint64_t size;        // PRIORIDAD: menor primero
    uint8_t  type;        // tipo: 0=jpg,1=png,2=gif,otros
} pqueue_entry_t;

// Inicializa la cola (llamar una vez en main)
int    pqueue_init(void);

// Libera la cola (opcional al final)
void   pqueue_destroy(void);

// Inserta copia ordenada por tamaño (menor primero)
int    pqueue_push(const pqueue_entry_t *entry);

// Retorna la cantidad de entradas en cola (solo informativo)
size_t pqueue_count(void);

void   pqueue_dump(void);

#endif // PQUEUE_H
