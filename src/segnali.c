/** @file signal.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione dei SEGNALI.
 */

#include <sys/signal.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "err_exit.h"
#include "segnali.h"

/**
 * inizializza un insieme di segnali per contenerli tutti
 * @param mySet - identificatore dell'insieme dei segnali
 */
void sig_fillset(sigset_t mySet) {
    if ((sigfillset(&mySet)) == -1)
        errExit("sigfillset failed");
}

/**
 * elimina dei segnali dal set passato come argomento
 * @param mySet - identificatore dell'insieme dei segnali
 * @param num - numero di segnali da rimuovere
 * @param ... - uno o più segnali da rimuovere
 */
void sig_remove(sigset_t mySet, int num, ...) {
    va_list valist;
    int i;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    /* access all the arguments assigned to valist */
    for (i = 0; i < num; i++) {
        sigdelset(&mySet,va_arg(valist, int));
    }

    /* clean memory reserved for valist */
    va_end(valist);

}

void sig_add(sigset_t mySet, int num, ...) {
    va_list valist;
    int i;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    /* access all the arguments assigned to valist */
    for (i = 0; i < num; i++) {
        sigaddset(&mySet,va_arg(valist, int));
    }

    /* clean memory reserved for valist */
    va_end(valist);

}

/**
 * imposta una maschera dei segnali per il processo corrente
 * @param flag - determina il cambiamento che la funzione apporta alla maschera
 * @param mySet - identificatore dell'insieme dei segnali per il processo corrente
 * @param oldSet - se non è nulla punta a una struttura che conterrà la maschera del
 * processo precedente alla chiamata di questa funzione
 */
void sig_setmask(int flag, sigset_t mySet, sigset_t oldSet) {
    if ((sigprocmask(flag, &mySet, &oldSet)) == -1)
        errExit("sigprocmask failed");
}

/**
 * imposta un nuovo handler per la ricezione di un determinato sengnale
 * @param signum - identificatore del segnale che verrà gestito dall'handler
 * @param handler - handler che andrà a gestire il segnale inviato all pprocesso
 */
void sig_sethandler(int signum, sighandler_t handler) {
    if ((signal(signum, handler)) == SIG_ERR)
        errExit("signal");
}
