/** @file shared_memory.h
 *  @brief Contiene la definizioni di variabili e funzioni
 *         specifiche per la gestione della MESSAGE QUEUE.
*/

#ifndef SYSTEM_CALL_MESSAGE_QUEUE_H
#define SYSTEM_CALL_MESSAGE_QUEUE_H

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <string.h>

#include "defines.h"

//#define MSG_LEN 1024
#define MSGSIZE (sizeof(struct mymsg) - sizeof(long)) //controllare primo sizeof
#define KEYMSQ 104


int alloc_message_queue(key_t msqKey);
int get_message_queue(key_t msqKey);
void msg_send(int msqid, void *msg, int msgflg);
void msg_receive(int msqid, void *msg, long msgtype, int msgflg);
void remove_message_queue(int msqid);

#endif //SYSTEM_CALL_MESSAGE_QUEUE_H
