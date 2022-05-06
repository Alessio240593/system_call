/** @file sender_manager.c
 *  @brief Contiene l'implementazione del sender_manager.
*/

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "defines.h"
#include "err_exit.h"

int main(void)
{
    char buffer[MAX_LEN];

    //create shmem
    int shmid = alloc_shared_memory(SHMKEY,SHMSIZE);
    char *shmem = (char *)attach_shared_memory(shmid, 0);

    //create and initialize semset
    int semid = alloc_semaphore(SEMKEY, SEMNUM);
    union semun arg;
    arg.val = SHMSEM;
    semctl(semid, 0, SETVAL, arg);

    //create fifo1
    make_fifo(FIFO1);

    //open fifo in read only mode
    int fd1 = open(FIFO1, O_RDONLY);
    SYSCHECK(fd1, "open");

    //read data from fifo1
    ssize_t bR = read(fd1, buffer, MAX_LEN);
    SYSCHECK(bR, "read");
    buffer[bR] = '\0';

    //from string to int
    int n = atoi(buffer);

    //costruzione messaggio da scrivere sulla shmem
    snprintf(buffer, sizeof(buffer), "Ho ricevuto %d file", n);

    //write data on shmem
    strcpy(shmem, buffer);

    //wake up client
    semOp(semid,0,1);
}
