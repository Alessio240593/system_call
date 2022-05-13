/** @file fifo.h
 *  @brief Contiene la definizioni di variabili e
 *         funzioni specifiche per la gestione delle FIFO.
*/

#ifndef FIFO_H
#define FIFO_H

#include <fifo.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>


void make_fifo(const char *path);
int open_fifo(const char *path, int mode);
void close_fd(int fd);
void remove_fifo(const char *path);
#endif
