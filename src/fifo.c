/** @file fifo.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione delle FIFO.
 */

#include <fifo.h>
#include <sys/stat.h>

#include "err_exit.h"

/**
 * Crea, se non esiste, una fifo
 * @param path - destinazione della fifo
 */
void make_fifo(const char *path)
{
    if (mkfifo(path, S_IRUSR | S_IWUSR) == -1)
        errExit("mkfifo failed");
}
