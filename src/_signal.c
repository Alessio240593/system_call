/** @file signal.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione dei SEGNALI.
 */

#include "err_exit.h"
#include "_signal.h"

/**
 * inizializza un insieme di segnali per contenerli tutti
 * @param mySet - identificatore dell'insieme dei segnali
 */
void sig_fillset(sigset_t *mySet)
{
    if ((sigfillset(mySet)) == -1)
        errExit("sigfillset failed: ");
}

/**
 * elimina dei segnali dal set passato come argomento
 * @param mySet - identificatore dell'insieme dei segnali
 * @param num - number of signals
 * @param ... - signals var_args
 */
void sig_remove(sigset_t *mySet, int num, ...)
{
    va_list list;
    va_start(list, num);

    for (int i = 0; i < num; i++) {
        if((sigdelset(mySet,va_arg(list, int)) == -1)) {
            errExit("sidelset failed: ");
        }
    }

    va_end(list);
}

/**
 * aggiunge dei segnali al set passato come argomento
 * @param mySet - identificatore dell'insieme dei segnali
 * @param num - numero di segnali da rimuovere
 * @param ... - uno o più segnali da rimuovere
 */
void sig_add(sigset_t *mySet, int num, ...)
{
    va_list valist;

    va_start(valist, num);

    for (int i = 0; i < num; i++) {
        if((sigaddset(mySet,va_arg(valist, int) == -1))) {
            errExit("siaddset failed: ");
        }
    }

    va_end(valist);
}

/**
 * imposta <myset> come maschera dei segnali per il processo corrente
 * @param flag - determina il cambiamento che la funzione apporta alla maschera
 * @param mySet - identificatore dell'insieme dei segnali per il processo corrente
 * processo precedente alla chiamata di questa funzione
 */
void sig_setmask(int flag, sigset_t *mySet)
{
    if ((sigprocmask(flag, mySet, 0)) == -1)
        errExit("sigprocmask failed: ");
}

/**
 * imposta <handler> come handler di default per la ricezione di un determinato sengnale
 * @param signum - identificatore del segnale che verrà gestito dall'handler
 * @param handler - handler che andrà a gestire il segnale inviato all pprocesso
 */
void sig_sethandler(int signum, sighandler_t handler)
{
    if ((signal(signum, handler)) == SIG_ERR)
        errExit("signal failed: ");
}
