/** @file segnali.h
 * @brief Contiene la definizioni di variabili e funzioni
 *        specifiche per la gestione dei SEGNALI.
*/

#ifndef SYSTEM_CALL__SIGNAL_H
#define SYSTEM_CALL__SIGNAL_H

#include <sys/signal.h>

//struttura per la gestione dei puntatori
typedef void (*sighandler_t)(int);

void sig_fillset(sigset_t mySet);
void sig_remove(sigset_t mySet, int sig, ...);
void sig_add(sigset_t mySet, int num, ...);
void sig_setmask(int flag, sigset_t mySet);
void sig_sethandler(int signum, sighandler_t handler);

#endif //SYSTEM_CALL__SIGNAL_H
