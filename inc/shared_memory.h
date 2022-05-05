/** @file shared_memory.h
 *  @brief Contiene la definizioni di variabili e funzioni
 *         specifiche per la gestione della MEMORIA CONDIVISA.
*/

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdlib.h>

// the Request structure defines a request sent by a client
struct Request {
    char pathname[250];
    key_t shmKey;
};

int alloc_shared_memory(key_t shmKey, size_t size);
int get_shared_memory(key_t shmKey, size_t size);
void *attach_shared_memory(int shmid, int shmflg);
void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid);

#endif
