/** @file defines.h
 * @brief Contiene la definizioni di variabili
 * e funzioni specifiche del progetto.
 */
#ifndef DEFINES_H
#define DEFINES_H

int check_string(const char* string1, char* string2);
int check_size(const char *path);
int count_char(int fd);
void sigusr1_handler(int sig);
void client_sigint_handler(int sig);
static int strCompare(const void *this, const void *other);
void getDirList(const char *startPath);
void fixDirList(void);

#endif
