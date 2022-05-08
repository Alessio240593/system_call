/** @file segnali.h
 * @brief Contiene la definizioni di variabili e funzioni
 *        specifiche per la gestione dei SEGNALI.
*/


#ifndef SYSTEM_CALL_SEGNALI_H
#define SYSTEM_CALL_SEGNALI_H

#include <signal.h>
#include "defines.h"

void sig_fillset(sigset_t mySet);
void sig_remove(sigset_t mySet, int sig, ...);
void sig_add(sigset_t mySet, int num, ...);
void sig_setmask(int flag, sigset_t mySet, sigset_t oldSet);
void sig_sethandler(int signum, sighandler_t handler);
#endif //SYSTEM_CALL_SEGNALI_H
