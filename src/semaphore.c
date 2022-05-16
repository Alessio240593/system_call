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
 * @return semid - id del set di semafori
 */
int alloc_semaphore(key_t semKey, int num)
{
    // get, or create, a shared memory segment
    int semid = semget(semKey, num, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR) ;

    if(semid == -1)
        errExit("semget failed: ");

    return semid;
}

/**
* Sincronizza il chiamante sul set di semafori specificato da semKey
* @param semKey - chiave del set di semafori
* @param num - numero di semafori del set su cui sincronizzarsi(deve corrispondere all'originale)
* @return semid - id del set di semafori
*/
int get_semaphore(key_t semKey, int num)
{
    int semid;
    semid = semget(semKey, num, S_IRUSR | S_IWUSR) ;

    if(semid == -1)
        errExit("semget failed: ");

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

    if (semop(semid, &sop, 1) == -1)
        errExit("semop failed: ");
}

/**
 * Esegue operazioni di controllo sul set di semafori
 * @param semid - id del set di semafori
 * @param semnum - numero del semaforo sul quale eseguire l'operazione
 * @param cmd - operazione di controllo
 */
 /*
void control_semaphore(int semid, int semnum, int cmd, int flag)
{
    if((semctl(semid, semnum, cmd, flag) == -1)) {
        errExit("semctl failed: ");
    }
}
*/
/**
 * Elimina il set di semafori
 * @param semid - id del set di semafori
 */
void remove_semaphore(int semid)
{
    if(semctl(semid, 0, IPC_RMID, 0) == -1)
        errExit("semctl failed: ");
    else
        printf("→ Semaphore set removed successfully!\n");
}

