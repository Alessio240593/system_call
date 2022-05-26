/**
 * @file semaphore.h
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

// TODO sistemare semafori attenzione ai risultati

//chiave e numero di semafori pr sincronizzazione
#define KEYSEM_SYNC 100
#define SEMNUM_SYNC 6

//operazioni sui semafori
#define WAIT -1
#define SYNC 0
#define SIGNAL 1

// 1° semaforo => mutex shared memory
// 2° semaforo => mutex message queue
// 3° semaforo => sincronizzare i client
// 4° semaforo => sincronizzare la lettura e scrittura shm
#define SEMSHM   0
#define SEMMSQ   1
#define SEMCHILD 2
#define SYNC_SHM 3
#define SYNC_FIFO1 4
#define SYNCH_MSQ 5
#define SEMFIFO1 6
#define SEMFIFO2 7

//chiave e numero di semafori per conteggio limite messaggi
#define KEYSEM_COUNTER 101
#define SEMNUM_COUNTER 3
// 1° semaforo => max messages in FIFO_1
// 2° semaforo => max messages in FIFO_2
// 3° semaforo => max messages in Message Queue
#define MAX_SEM_FIFO1 0
#define MAX_SEM_FIFO2 1
#define MAX_SEM_MSQ   2


/**
 * Struttura per lavorare con semctl
 */
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);
int semOp_no_wait (int semid, unsigned short sem_num, short sem_op);
int alloc_semaphore(key_t semKey, int num);
int get_semaphore(key_t semKey, int num);
void remove_semaphore(int shmid);

#endif

