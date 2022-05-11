/** @file sender_manager.c
 *  @brief Contiene l'implementazione del sender_manager.
*/

#include "fifo.h"
#include "_signal.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "defines.h"
#include "message_queue.h"

extern const char *signame[];

int fd1 = -1;
int shmid = -1;
int semid = -1;
int fd2 = -1;
int msqid = -1;
char *shmem = NULL;

void sigint_handler(int sig)
{
    int ret;
    printf(" → <Server>: Received signal %s\n", signame[sig]);

    if(fd1 >= 0) {
        ret = close(fd1);
        SYSCHECK_V(ret, "close");
        ret = unlink(FIFO1);
        SYSCHECK_V(ret, "unlink");
    }

    if(fd2 >= 0) {
        ret = close(fd2);
        SYSCHECK_V(ret, "close");
        ret = unlink(FIFO1);
        SYSCHECK_V(ret, "unlink");
    }


    if(shmid >= 0 && shmem != NULL) {
        free_shared_memory(shmem);
        remove_shared_memory(shmid);
    }

    if(semid >= 0)
        remove_semaphore(semid);

    if(msqid >= 0)
        remove_message_queue(msqid);

    exit(EXIT_SUCCESS);

}

int main(void)
{
    char buffer[MAX_LEN];

    // create shmem
    shmid = alloc_shared_memory(KEYSHM,SHMSIZE);
    shmem = (char *)attach_shared_memory(shmid, 0);

    // create and initialize semaphore set
    semid = alloc_semaphore(KEYSEM, SEMNUM);
    union semun arg;
    arg.val = 0;
    // set sem1 and sem2 at 0
    semctl(semid, SEMSHM, SETVAL, arg);
    semctl(semid, SEMMSQ, SETVAL, arg);
    // create semaphore of max messages in msg_queue
    arg.val = 50;
    semctl(semid, SEMCOUNT, SETVAL, arg);
    // create semaphore to sync clients
    arg.val = 1;
    semctl(semid, SEMCHILD, SETVAL, arg);

    // create message queue
    msqid = alloc_message_queue(KEYMSQ);

    // create FIFOs
    make_fifo(FIFO1);
    make_fifo(FIFO2);

    sig_sethandler(SIGINT, sigint_handler);

    // open fifo1 in read only mode
    while (1) {

        fd1 = open(FIFO1, O_RDONLY);
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
}
