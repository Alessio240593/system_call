/** @file defines.h
 * @brief Contiene la definizioni di variabili
 *        e funzioni specifiche del progetto.
 */
#ifndef DEFINES_H
#define DEFINES_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_FILE_SIZE 4096
#define MAX_PATH 4096
#define MAX_LEN 512
#define LEN_INT 11
#define PARTS 4

// FIFOs path
#define FIFO1 "/tmp/myDir/fifo1"
#define FIFO2 "/tmp/myDir/fifo2"

#define MCHECK(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return 1;       \
                     }

#define MCHECK_V(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return;       \
                     }

#define SYSCHECK(ret, type)     if (ret == -1){  \
                                    if(errno == EACCES)  \
                                        printf("fifo not exist");  \
                                    else         \
                                        perror(type); \
                                return 1;      \
                                }

#define SYSCHECK_V(ret, type)  if (ret == -1) { \
                                perror(type); \
                                return;       \
                            }
#define WCHECK(ret,dim)  if (ret != dim) { \
                                perror("write"); \
                                return 1;       \
                            }

#define WCHECK_V(ret,dim)  if (ret != dim) { \
                                perror("write"); \
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
int split_file(char** parts, int fd, size_t tot_char);

int init_dirlist(dirlist_t *dirlist, const char *start_path);
int dump_dirlist(dirlist_t *dirlist, const char *filename);

#endif
