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
 * Apre una fifo nella modalità passata da <mode>
 * @param path - fifo path
 * @param mode - modalità di apertura della fifo
 * @return fd - file descriptor associato alla fifo
 */
int open_fifo(const char *path, int mode)
{
    errno = 0;
    int fd = open(path, mode);

    if(errno == EACCES)
        errExit("fifo not exist");

    return fd;
}

/**
 * Scrivi sulla fifo <size> byte
 * @param fd - fifo file descriptor
 * @param buf - buffer contenente i dati da scrivere
 * @param size - byte totali da scrivere
 * @return Br - numero di byte scritti
 */
ssize_t write_fifo(int fd, void *buf, ssize_t size)
{
    ssize_t Bw = write(fd, buf, size);

    if (Bw != size) {
        errExit("write failed: ");
    }

    return Bw;
}

/**
 * Leggi dalla fifo <size> byte
 * @param fd - fifo file descriptor
 * @param buf - buffer che conterrà i dati letti dalla fifo
 * @param size - byte totali da leggere
 * @return Br - numero di byte letti
 */
ssize_t read_fifo(int fd, void *buf, ssize_t size)
{
    ssize_t Br = read(fd, buf, size);

    if (Br == -1) {
        errExit("write failed: ");
    }

    return Br;
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