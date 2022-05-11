/** @file fifo.c
 *  @brief Contiene l'implementazione delle funzioni
 *         specifiche per la gestione delle FIFO.
 */

#include "fifo.h"
#include "err_exit.h"

/**
 * Crea, se non esiste, una fifo
 * @param path - destinazione della fifo
 */
void make_fifo(const char *path)
{
    if (mkfifo(path, S_IRUSR | S_IWUSR) == -1)
        errExit("mkfifo failed: ");
}

/**
 * Chiude il file descriptor associato alla fifo
 * @param fd - file descriptor associato alla fifo
 */
void close_fd(int fd)
{
    if(close(fd) == -1)
        errExit("close failed: ");
}

/**
 * Rimuove la fifo <path>
 * @param path - path della fifo
 */
void remove_fifo(const char *path)
{
    if(unlink(path) == -1)
        errExit("unlink failed: ");

    printf("→ Fifo removed successfully!\n");

}