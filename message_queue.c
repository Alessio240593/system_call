/** @file shared_memory.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione della MESSAGE QUEUE.
*/

#include "message_queue.h"
#include "err_exit.h"

/**
 * Crea, se non esiste, un coda di messaggi
 * @param msqKey - chiave per creare la coda di messaggi
 * @return msqid - id della coda di messaggi
 */
int alloc_message_queue(key_t msqKey)
{
    int msqid;

    msqid = msgget(msqKey, IPC_CREAT | S_IRUSR | S_IWUSR | IPC_EXCL) ;

    if(msqid == -1)
        errExit("msgget failed: ");

    printf("→ <Server>: Message queue create successfully!\n");

    return msqid;
}

/**
* Sincronizza il chiamante sulla message queue specificata da msqKey
* @param msqKey - chiave della message queue
* @return msqid - id della coda di messaggi
*/
int get_message_queue(key_t msqKey)
{
    int msqid;

    msqid = msgget(msqKey, S_IRUSR | S_IWUSR) ;

    if(msqid == -1)
        errExit("msgget failed: ");

    printf("→ <Client0>: Message queue synchronization successfully!\n");

    return msqid;
}

/**
 * Rimuove la coda di messaggi
 * @param msqid - id della coda di messaggi
 */
void remove_message_queue(int msqid)
{
    if(msgctl(msqid, IPC_RMID, NULL) == -1)
        errExit("msgctl failed: ");
    else
        printf("→ <Server>: Message queue removed successfully!\n");
}
