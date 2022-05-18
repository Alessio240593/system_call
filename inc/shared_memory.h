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

#include "defines.h"

#define SHMSIZE MAXMSG * sizeof(msg_t) //controllare sizeof
#define KEYSHM 100

int alloc_shared_memory(key_t shmKey, size_t size);
int get_shared_memory(key_t shmKey, size_t size);
void *attach_shared_memory(int shmid, int shmflg);
void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid);
int shmem_add(msg_t **dest, msg_t *src);
int there_is_message(msg_t **shmem);
int is_empty(msg_t **shmem, size_t index);

#endif
