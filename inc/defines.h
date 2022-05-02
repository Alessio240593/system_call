/** @file defines.h
 * @brief Contiene la definizioni di variabili
 *        e funzioni specifiche del progetto.
 */
#ifndef DEFINES_H
#define DEFINES_H

#include <stdlib.h>

#define MAX_FILE_SIZE 4096
#define MAX_PATH 4096

#define MCHECK(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return 1;       \
                     }

#define MCHECK_V(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return;       \
                     }

typedef struct __dirlist_t {
    char **list;
    size_t index;
    size_t size;
} dirlist_t;

int check_string(const char *string1, char *string2);
int check_size(const char *path);
ssize_t count_char(int fd);
int is_dir(const char *_path);
void sigusr1_handler(int sig);
void sigint_handler(int sig);
int Chdir(const char *path);
int init_dirlist(dirlist_t *dirlist, const char *start_path);
int dump_dirlist(dirlist_t *dirlist, const char *filename);


#endif
