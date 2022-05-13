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

int fifo_flag = 0;
int fd1 = -1;
int shmid = -1;
int semid = -1;
int fd2 = -1;
int msqid = -1;
msg_t *shmem = NULL;

void sigint_handler(int sig)
{
    printf("\t→ <Server>: Received signal %s\n\n", signame[sig]);

    if(fd1 >= 0) {
        close_fd(fd1);
    }

    if(fifo_flag){
        remove_fifo(FIFO1);
        fifo_flag = 0;
    }

    if(fd2 >= 0) {
        close_fd(fd2);
        remove_fifo(FIFO2);
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
    shmem = (msg_t *)attach_shared_memory(shmid, 0);

    // create and initialize semaphore set
    semid = alloc_semaphore(KEYSEM_SYNC, SEMNUM_SYNC);
    union semun arg;
    arg.val = 0;
    // set sem1 and sem2 at 0
    semctl(semid, SEMSHM, SETVAL, arg);
    semctl(semid, SEMMSQ, SETVAL, arg);
    // create semaphore to sync clients
    arg.val = 1;
    semctl(semid, SEMCHILD, SETVAL, arg);

    // create message queue
    msqid = alloc_message_queue(KEYMSQ);

    // create FIFOs
    make_fifo(FIFO1);
    //make_fifo(FIFO2);

    fifo_flag = 1;

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
        snprintf(buffer, sizeof(buffer), "\t← <Server>: Sono pronto per la ricezione di %d file\n\n", n);

        //write data on shmem
        //strcpy(shmem, buffer);
        shmem->message = strdup(buffer);

        //wake up client
        semOp(semid,0,SIGNAL);
    }
}
