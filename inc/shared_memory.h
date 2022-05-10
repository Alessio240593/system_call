/** @file shared_memory.h
 *  @brief Contiene la definizioni di variabili e funzioni
 *         specifiche per la gestione della MEMORIA CONDIVISA.
*/

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMSIZE 4096 * 100
#define KEYSHM 100 //change value

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
