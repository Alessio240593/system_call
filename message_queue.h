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

#define MSGSIZE (sizeof(struct __msg_t) - sizeof(long))
#define KEYMSQ 104

int alloc_message_queue(key_t msqKey);
int get_message_queue(key_t msqKey);
void remove_message_queue(int msqid);

#endif //SYSTEM_CALL_MESSAGE_QUEUE_H
