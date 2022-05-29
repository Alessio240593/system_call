/**
 * @file defines.h
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
#include <linux/limits.h>

#define MAX_FILE_SIZE 4096
#define MAX_LEN 512
#define LEN_INT 11
#define PARTS 4
#define MAXMSG 50
#define MAX_FILE 100

// FIFOs path
#define FIFO_1 get_FIFO_1()
#define FIFO_2 get_FIFO_2()

// define IPCs index
#define FIFO1 0
#define FIFO2 1
#define MSQ   2
#define SHM   3


#define MCHECK(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return 1;       \
                     }
// TODO da controllare
#define SYSCHECK(ret, type)     if (ret == -1){  \
                                    perror(type); \
                                    return 1;      \
                                }

typedef struct __dirlist_t {
    size_t index;
    size_t size;
    char **list;
} dirlist_t;

typedef struct __msg_t {
    long type;

    size_t client;
    pid_t pid;
    char name[MAX_LEN];
    char message[MAX_LEN];
} msg_t;


char *get_FIFO_1(void);
char *get_FIFO_2(void);
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
int child_finish(int matrice[37][4], size_t child);
int ends_with(const char *str, const char *end);
char *append_out(const char *str);
//char* parts_header(int part, const char *path, pid_t pid);

#endif