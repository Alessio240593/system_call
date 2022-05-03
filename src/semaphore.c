/** @file semaphore.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione dei SEMAFORI.
 */

#include "err_exit.h"
#include "semaphore.h"
#include <sys/sem.h>
#include <sys/stat.h>

void semOp (int semid, unsigned short sem_num, short sem_op)
{
    struct sembuf sop = {.sem_num = sem_num, . sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        errExit("semop failed");
}


int alloc_semaphore(key_t semKey, int num)
{
    // get, or create, a shared memory segment
    int semid = semget(semKey, num, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR) ;

    if(semid == -1)
        errExit("semget failed");

    return semid;
}

/*
void control_semaphore(int semid)
{
    // attach the shared memory
    int *ptr = semctl(shmid, NULL, shmflg);

    if(ptr == (void *)-1)
        errExit("shmat failed");

}
*/

void remove_semaphore(int semid)
{
    if(semctl(semid, 0, IPC_RMID, 0) == -1)
        errExit("semctl failed");
}

