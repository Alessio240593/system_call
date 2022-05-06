/** @file semaphore.h
 * @brief Contiene la definizioni di variabili e funzioni
 *        specifiche per la gestione dei SEMAFORI.
*/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdlib.h>
#include <sys/sem.h>

#define SEMKEY 100 //change value
#define SEMNUM 1

/**
 * Struttura per lavorare con semctl
 */
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);
int alloc_semaphore(key_t semKey, int num);
int get_semaphore(key_t semKey, int num);
void control_semaphore(int semid);
void remove_semaphore(int shmid);

#endif

