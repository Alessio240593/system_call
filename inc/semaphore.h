/** @file semaphore.h
 * @brief Contiene la definizioni di variabili e funzioni
 *        specifiche per la gestione dei SEMAFORI.
*/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdio.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/sem.h>

#include "defines.h"


#define KEYSEM_SYNC 100 //change value
#define SEMNUM_SYNC 3
#define KEYSEM_COUNTER 101
#define SEMNUM_COUNTER 4

//operazioni sui semafori
#define WAIT -1
#define SYNC 0
#define SIGNAL 1

// 1° semaforo => mutex shared memory
// 2° semaforo => mutex message queue
// 3° semaforo => sincronizzare i client
#define SEMSHM   0
#define SEMMSQ   1
#define SEMCHILD 2

// 1° semaforo => max messages in FIFO1
// 2° semaforo => max messages in FIFO2
// 3° semaforo => max messages in Shared Memory
// 4° semaforo => max messages in Message Queue
#define MAX_SEM_FIFO1 0
#define MAX_SEM_FIFO2 1
#define MAX_SEM_MSQ   2
#define MAX_SEM_SHM   3

/**
 * Struttura per lavorare con semctl
 */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);
int alloc_semaphore(key_t semKey, int num);
int get_semaphore(key_t semKey, int num);
void control_semaphore(int semid, int semnum, int cmd, int flag);
void remove_semaphore(int shmid);

#endif

