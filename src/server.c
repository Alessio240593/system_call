/** @file sender_manager.c
 *  @brief Contiene l'implementazione del sender_manager.
*/

#include <sys/stat.h>
#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <string.h>

int main(/*int argc, char* argv[]*/){

    printf("hello");
    if (mkfifo(FIFO1, S_IRUSR | S_IWUSR) == -1)
        errExit("mkfifo failed");
    printf("Mike");

    int fd1 = open(FIFO1, O_RDONLY);
    SYSCHECK(fd1, "open");

    char buffer[MAX_LEN];

    ssize_t bR = read(fd1, buffer, MAX_LEN);
    SYSCHECK(bR, "read");
    buffer[bR] = '\0';

    int n = atoi(buffer);

    key_t shmKey = ftok(FIFO1, 'a');
    int shmid = alloc_shared_memory(shmKey,SHMSIZE);

    char *shmem = (char *)get_shared_memory(shmid, 0);

    //costruzione messaggio da scrivere sulla shmemory
    snprintf(buffer, sizeof(buffer), "Ho ricevuto gli %d file", n);

    strcpy(shmem, buffer);

    key_t semKey = ftok(FIFO1, 'b');
    int semid = alloc_semaphore(semKey, 1);
    union semun arg;
    arg.val = 0;
    semctl(semid, 0, SETVAL, arg);

    semOp(semid,0,1);


}
