/** @file semaphore.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione dei SEMAFORI.
 */

#include "semaphore.h"
#include "err_exit.h"

/**
 * Crea, se non esiste, un set di semafori
 * @param semKey - chiave per creare il set di semafori
 * @param num - numero di semafori da creare
 * @return semid_sync - id del set di semafori
 */
int alloc_semaphore(key_t semKey, int num)
{
    // get, or create, a shared memory segment
    int semid = semget(semKey, num, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR) ;

    if(semid == -1)
        errExit("semget failed: ");

    printf("→ <Server>: Semaphore set create successfully!\n");

    return semid;
}

/**
* Sincronizza il chiamante sul set di semafori specificato da semKey
* @param semKey - chiave del set di semafori
* @param num - numero di semafori del set su cui sincronizzarsi(deve corrispondere all'originale)
* @return semid_sync - id del set di semafori
*/
int get_semaphore(key_t semKey, int num)
{
    int semid;
    semid = semget(semKey, num, S_IRUSR | S_IWUSR) ;

    if(semid == -1)
        errExit("semget failed: ");

    printf("→ <Client0>: Semaphore set synchronization successfully!\n");

    return semid;
}

/**
 * Esegue un operazione sul set di semafori
 * @param semid - id del set di semafori
 * @param sem_num - numero del semaforo su cui eseguire l'operazione
 * @param sem_op - tipo di operazione
 */
void semOp (int semid, unsigned short sem_num, short sem_op)
{
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    int res = semop(semid, &sop, 1);

if(res == -1){
        printf("semaforo numero: %d\n", sem_num);
        errExit("semop failed: ");
    }
}


/**
 * Esegue un operazione non bloccante sul set di semafori
 * @param semid - id del set di semafori
 * @param sem_num - numero del semaforo su cui eseguire l'operazione
 * @param sem_op - tipo di operazione
 */
int semOp_no_wait (int semid, unsigned short sem_num, short sem_op)
{
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = IPC_NOWAIT};

    errno = 0;

    int res = semop(semid, &sop, 1);

    if(res == -1 && errno == EAGAIN)
    {
        //printf("→ <Server>: %s occupata!\n", sem_num == SEMSHM ? "Shared memory" : "Semaphore set");
        return EAGAIN;
    }
    else if(res == -1){
        printf("semaforo numero: %d: ", sem_num);
        errExit("semop failed: ");
    }

    return 0;
}

/**
 * Elimina il set di semafori
 * @param semid - id del set di semafori
 */
void remove_semaphore(int semid)
{
    if(semctl(semid, 0, IPC_RMID, 0) == -1)
        errExit("semctl failed: ");
    else
        printf("→ <Server>: Semaphore set removed successfully!\n");
}

