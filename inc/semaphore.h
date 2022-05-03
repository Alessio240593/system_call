/** @file semaphore.h
 * @brief Contiene la definizioni di variabili e funzioni
 *        specifiche per la gestione dei SEMAFORI.
*/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <stdlib.h>
#include <sys/sem.h>

//insert this structure in a header file <sem.h>
// definition of the union semun
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

/*
struct sembuf {
    unsigned short sem_num; // Semaphore number //
    short sem_op;
    // Operation to be performed //
    short sem_flg;
    // Operation flags //
};
*/

/* errsemOpExit is a support function to manipulate a semaphore's value
 * of a semaphore set. semid is a semaphore set identifier, sem_num is the
 * index of a semaphore in the set, sem_op is the operation performed on sem_num
 */
void semOp (int semid, unsigned short sem_num, short sem_op);
int alloc_semaphore(key_t semKey, int num);
void control_semaphore(int semid);
void remove_semaphore(int shmid);


#endif

