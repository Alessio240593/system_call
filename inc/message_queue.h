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

#define TEXTSIZE 1024
#define MSGSIZE (sizeof(struct mymsg) - sizeof(long))
#define MSGSEM 1
#define MSGKEY 104
#define MAXMSQ 50

struct mymsg {
    long type;
    char text[TEXTSIZE];
};

int alloc_message_queue(key_t msqKey);
int get_message_queue(key_t msqKey);
void fill_msg(struct mymsg *msg, long type, const char *text);
void msg_send(int msqid, const void *msg, int msgflg);
void msg_receive(int msqid, void *msg, long msgtype, int msgflg);
void remove_message_queue(int msqid);

#endif //SYSTEM_CALL_MESSAGE_QUEUE_H
