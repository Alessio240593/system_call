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
ssize_t write_fifo(int fd, void *buf, ssize_t size);
ssize_t read_fifo(int fd, int num, void *buf, ssize_t size);
void close_fd(int fd);
void remove_fifo(const char *path, int num);
#endif
