/** @file shared_memory.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione della MEMORIA CONDIVISA.
*/

#include "err_exit.h"
#include "shared_memory.h"
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/**
 * Crea, se non esiste, un segmento di memoria condivisa
 * @param shmKey - chiave per creare a shared memory
 * @param size - dimensione zona di memoria
 * @return shmid - id del segmento di memoria
 */
int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int shmid;
    shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR | IPC_EXCL) ;

    if(shmid == -1)
        errExit("shmget failed");

    return shmid;
}

/**
 * Link della zona di memoria nello spazio di indirizzamento logico del processo
 * @param shmid - id del segmento di memoria
 * @param shmflg - flag per la system call
 * @return void * - puntatore alla zona di memoria condivisa
 */
void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    int *ptr = shmat(shmid, NULL, shmflg);

    if(ptr == (void *)-1)
        errExit("shmat failed");

    return ptr;
}

/**
 * Unlink della zona di memoria nello spazio di indirizzamento logico del processo
 * @param ptr_sh - puntatore alla zona di memoria condivisa
 */
void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if(shmdt(ptr_sh) == -1)
        errExit("shmdt failed");
}

/**
 * Segna la zona di memoria come removibile, se il processo è l'ultimo ad eseguire
 * la detach, allora l'area di memoria viene eliminata
 * @param semid - id alla zona di memoria condivisa
 */
void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl failed");
}

