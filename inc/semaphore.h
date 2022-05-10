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

#define KEYSEM 100 //change value
#define SEMNUM 4

#define WAIT -1
#define SYNC 0
#define SIGNAL 1

#define SEMSHM 0
#define SEMMSQ 1
#define SEMCOUNT 2
#define SEMCHILD 3
// 1° semaforo => shared memory
// 2° semaforo => message queue
// 3° semaforo => max messages in msg_queue
// 4° semaforo => sincronizzare i client

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

