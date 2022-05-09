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

    return msqid;
}

/**
 * Prepara il messaggio da inviare
 * @param msg - struttura che conterrà il messaggio
 * @param type - tipo di messaggio
 * @param text - testo del messaggio
 */
void fill_msg(struct mymsg *msg, long type, const char *text)
{
    msg->type = type;
    strncpy(msg->text, text, MSGSIZE);
}

/**
 * Invia msg alla coda di messaggi, con la flag IPC_NOWAIT questa syscall viene eseguita in modalità polling
 * @param msqid - id della coda di messaggi
 * @param msg - messaggio da inviare
 * @param msgflg - flag
 */
void msg_send(int msqid, const void *msg, int msgflg)
{
    if (msgsnd(msqid, msg, MSGSIZE, msgflg) == -1)
        errExit("msgsnd failed: ");
}

/**
 * Legge e rimuove un messaggio dalla coda di messaggi, con la flag IPC_NOWAIT questa syscall viene eseguita in modalità polling
 * @param msqid - id della coda di messaggi
 * @param msg - messaggio da inviare
 * @param msgtype - tipo del mesaggio : 0: il primo messaggio viene tolto dalla coda
                                        > 0: il primo messaggio del valore corrispondente viene rimosso dalla coda
                                        < 0: il primo messaggio con valore assoluto minore o uguale viene rimosso dalla coda
 * @param msgflg - flag
 */
void msg_receive(int msqid, void *msg, long msgtype, int msgflg)
{
    if (msgrcv(msqid, msg, MSGSIZE, msgtype , msgflg) == -1)
        errExit("msgrcv failed: ");
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
        printf("Message queue removed successfully\n");
}
