/** @file sender_manager.c
 *  @brief Contiene l'implementazione del sender_manager.
*/

#include "fifo.h"
#include "_signal.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "defines.h"
#include "message_queue.h"

int main(void)
{
    char buffer[MAX_LEN];

    // create shmem
    int shmid = alloc_shared_memory(SHMKEY,SHMSIZE);
    char *shmem = (char *)attach_shared_memory(shmid, 0);

    // create and initialize semaphore set
    int semid = alloc_semaphore(SEMKEY, SEMNUM);
    union semun arg;
    arg.val = 0;
    // set sem1 and sem2 at 0
    semctl(semid, SHMSEM, SETVAL, arg);
    semctl(semid, MSGSEM, SETVAL, arg);
    // create semaphore of max messages in msg_queue
    arg.val = 50;
    semctl(semid, COUNTSEM, SETVAL, arg);
    // create semaphore to sync clients
    arg.val = 1;
    semctl(semid, CHILDSEM, SETVAL, arg);

    // create message queue
    //int msqid = alloc_message_queue(MSGKEY);

    // create FIFOs
    make_fifo(FIFO1);
    make_fifo(FIFO2);

    // open fifo1 in read only mode
    int fd1 = open(FIFO1, O_RDONLY);
    SYSCHECK(fd1, "open");

    //read data from fifo1
    ssize_t bR = read(fd1, buffer, MAX_LEN);
    SYSCHECK(bR, "read");
    buffer[bR] = '\0';

    //from string to int
    int n = atoi(buffer);

    //costruzione messaggio da scrivere sulla shmem
    snprintf(buffer, sizeof(buffer), "<Server>: Ho ricevuto %d file\n", n);

    //write data on shmem
    strcpy(shmem, buffer);

    //wake up client
    semOp(semid,0,SIGNAL);
}
