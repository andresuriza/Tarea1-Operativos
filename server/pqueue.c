#include "pqueue.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  

typedef struct {
    pqueue_entry_t *a;
    size_t n, cap;
    pthread_mutex_t m;
} pqueue_t;

static pqueue_t Q;

int pqueue_init(void){
    memset(&Q, 0, sizeof Q);
    if (pthread_mutex_init(&Q.m, NULL) != 0) return -1;
    return 0;
}

void pqueue_destroy(void){
    pthread_mutex_lock(&Q.m);
    free(Q.a); Q.a=NULL; Q.n=Q.cap=0;
    pthread_mutex_unlock(&Q.m);
    pthread_mutex_destroy(&Q.m);
}

static int ensure_cap(size_t need){
    if (Q.cap >= need) return 0;
    size_t nc = Q.cap ? Q.cap * 2 : 64;
    if (nc < need) nc = need;
    pqueue_entry_t *na = realloc(Q.a, nc * sizeof(pqueue_entry_t));
    if (!na) return -1;
    Q.a = na; Q.cap = nc;
    return 0;
}

int pqueue_push(const pqueue_entry_t *entry){
    if (!entry) return -1;
    pthread_mutex_lock(&Q.m);
    if (ensure_cap(Q.n + 1) != 0) {
        pthread_mutex_unlock(&Q.m);
        return -1;
    }

    // Inserción ordenada por tamaño ascendente
    size_t pos = Q.n;
    while (pos > 0 && Q.a[pos-1].size > entry->size) {
        Q.a[pos] = Q.a[pos-1];
        pos--;
    }
    Q.a[pos] = *entry;
    Q.n++;

    pthread_mutex_unlock(&Q.m);
    return 0;
}

size_t pqueue_count(void){
    pthread_mutex_lock(&Q.m);
    size_t n = Q.n;
    pthread_mutex_unlock(&Q.m);
    return n;
}
void pqueue_dump(void){
    pthread_mutex_lock(&Q.m);
    printf("[pqueue] total=%zu\n", Q.n);
    for (size_t i = 0; i < Q.n; i++){
        printf("  [%zu] %s (%llu B) type=%u -> %s\n",
               i,
               Q.a[i].name,
               (unsigned long long)Q.a[i].size,
               (unsigned)Q.a[i].type,
               Q.a[i].path);
    }
    pthread_mutex_unlock(&Q.m);
}
void pqueue_clear(void){
    pthread_mutex_lock(&Q.m);
    Q.n = 0;                  // vacía (no libera memoria)
    pthread_mutex_unlock(&Q.m);
}