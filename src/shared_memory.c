/** @file shared_memory.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione della MEMORIA CONDIVISA.
*/

#include "shared_memory.h"
#include "err_exit.h"

/**
 * Crea, se non esiste, un segmento di memoria condivisa
 * @param shmKey - chiave per creare la shared memory
 * @param size - dimensione zona di memoria
 * @return shmid - id del segmento di memoria
 */
int alloc_shared_memory(key_t shmKey, size_t size)
{
    int shmid;
    shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR | IPC_EXCL) ;

    if(shmid == -1)
        errExit("shmget failed: ");

    return shmid;
}

/**
* Sincronizza il chiamante sulla shared memory specificata da shmKey
* @param shmKey - chiave della shared memory
* @param size - dimensione zona di memoria(deve essere <= alla shared memory già esistente)
* @return shmid - id del segmento di memoria
*/
int get_shared_memory(key_t shmKey, size_t size)
{
    int shmid;
    shmid = shmget(shmKey, size, S_IRUSR | S_IWUSR) ;

    if(shmid == -1)
        errExit("shmget failed: ");

    return shmid;
}

/**
 * Link della zona di memoria nello spazio di indirizzamento logico del processo
 * @param shmid - id del segmento di memoria
 * @param shmflg - flag per la system call
 * @return void * - puntatore alla zona di memoria condivisa
 */
void *attach_shared_memory(int shmid, int shmflg)
{
    int *ptr = shmat(shmid, NULL, shmflg);

    if(ptr == (void *)-1)
        errExit("shmat failed: ");

    return ptr;
}

/**
 * Unlink della zona di memoria nello spazio di indirizzamento logico del processo
 * @param ptr_sh - puntatore alla zona di memoria condivisa
 */
void free_shared_memory(void *ptr_sh)
{
    if(shmdt(ptr_sh) == -1)
        errExit("shmdt failed: ");
}

/**
 * Segna la zona di memoria come removibile, se il processo è l'ultimo ad eseguire
 * la detach, allora l'area di memoria viene eliminata
 * @param semid - id alla zona di memoria condivisa
 */
void remove_shared_memory(int shmid)
{
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl failed: ");
    else
        printf("→ Shared memory segment removed successfully!\n");
}
// TODO
// unico vettore di msg_t o vettore di supporto?
int shmem_add(msg_t *dest, const msg_t src)
{
    size_t i;
    int ret = 1;

    for (i = 0; i < MAXMSG; i++) {
        if ((dest + i) == NULL) {
            memcpy((dest + i), &src, GET_MSG_SIZE(src));
            ret = 0;
            break;
        }
    }

    return ret;

    /*void write(msg_t **set_msg, msg_t *msg){
   for(int i = 0; i < 50; i++){
    if(set_msg[i]->txt == NULL){
      memcpy(set_msg[i], msg, sizeof(msg_t));
      break;
    }
    else{
      printf("\t→posizione %d occupata\n", (i + 1));
    }
  }
  return;
    }*/
}

/**
 *  Controlla se ci sono messaggi <msg_t> nella shared memory
 * @param shmem - segmento di memoria condivisa
 * @return 0 - se non ci sono messaggi in memoria
 * @return 1 - se c'è almeno un messaggio in memoria
 */
int there_is_message(msg_t **shmem)
{
    size_t i;

    for (i = 0; i < MAXMSG; i++) {
        if(shmem[i]->message != NULL){
            return 1;
        }
    }
    return 0;
}

/**
 * Controlla se la posizione <index> della shared memory è vuota
 * @param shmem - segmento di memoria condivisa
 * @param index - posizione del vettore di memoria condivisa da controllare
 * @return 0 - se alla posizione index la shared memory non è vuota
 * @return 1 - se alla posizione index la shared memory è vuota
 */
int is_empty(msg_t **shmem, int index)
{
    return shmem[index]->message == NULL;
}

// TODO
//controllare se la riga relativa al processo i ha tutte 4 le parti
/**
 *
 * @param shmem_array
 * @param index
 * @return
 */
 /*
int prova(msg_t **shmem_array, int index)
{
    for (int i = 0; i < 4; ++i) {
        if(shmem_array[index][i]->message == NULL) {
            return 0;
        }
    }
    return 1;
}
*/