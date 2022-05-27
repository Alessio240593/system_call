/** @file err_exit.c
 *  @brief Contiene l'implementazione della funzione di stampa degli errori.
 */

#include "err_exit.h"

/**
 * Stampa il messaggio di errore <msg> e ritorna il codice d'errore EXIT_FAILURE
 * @param msg - messaggio da stampare
 */
void errExit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
