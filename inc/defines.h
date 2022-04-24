/** @file defines.h
 * @brief Contiene la definizioni di variabili
 *        e funzioni specifiche del progetto.
 */
#ifndef DEFINES_H
#define DEFINES_H

#include <unistd.h>

#define MAX_PATH       512
#define MAX_FILE_SIZE 4096

int check_string(const char* string1, char* string2);
int check_size(const char *path);
ssize_t count_char(int fd);
int is_dir(const char *_path);
void sigusr1_handler(int sig);
void sigint_handler(int sig);
int strCompare(const void *this, const void *other);
void getDirList(const char *startPath);
void fixDirList(void);
void dumpDirList(const char *filename);


#endif
