/** @file sender_manager.c
 *  @brief Contiene l'implementazione del sender_manager.
*/

#include "fifo.h"
#include "_signal.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "defines.h"
#include "message_queue.h"
#include "err_exit.h"

extern const char *signame[];

int fifo_flag = 0;
int fd1 = -1;
int shmid = -1;
int semid = -1;
int semid_counter = -1;
int fd2 = -1;
int msqid = -1;
msg_t **shmem = NULL;

void sigint_handler(int sig)
{
    printf("\t→ <Server>: Received signal %s\n\n", signame[sig]);

    if(fd1 >= 0) {
        close_fd(fd1);
    }

    if(fd2 >= 0) {
        close_fd(fd2);
        remove_fifo(FIFO2);
    }

    if(fifo_flag){
        remove_fifo(FIFO1);
        remove_fifo(FIFO2);
        fifo_flag = 0;
    }

    if(shmid >= 0 && shmem != NULL) {
        free_shared_memory(shmem);
        remove_shared_memory(shmid);
    }

    if(semid >= 0)
        remove_semaphore(semid);

    if(semid_counter >= 0)
        remove_semaphore(semid_counter);

    if(msqid >= 0)
        remove_message_queue(msqid);

    exit(EXIT_SUCCESS);

}

int main(void)
{
    char buffer[MAX_LEN];

    // create shmem
    shmid = alloc_shared_memory(KEYSHM,SHMSIZE);
    shmem = (msg_t **)attach_shared_memory(shmid, 0);

    // Create a semaphore with 50 max messages
    semid_counter = alloc_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);
    unsigned short values[] = { MAXMSG, MAXMSG, MAXMSG, MAXMSG};
    union semun arg;
    arg.array = values;
    semctl(semid_counter, 0, SETALL, arg);

    //create synch server
    semid = alloc_semaphore(KEYSEM_SYNC, SEMNUM_SYNC);
    arg.val = 1;
    // set sem1 and sem2 at 1
    semctl(semid, SEMSHM, SETVAL, arg);
    semctl(semid, SEMMSQ, SETVAL, arg);
    // create semaphore to sync clients
    semctl(semid, SEMCHILD, SETVAL, arg);

    // create message queue
    msqid = alloc_message_queue(KEYMSQ);

    // create FIFOs
    make_fifo(FIFO1);
    make_fifo(FIFO2);

    fifo_flag = 1;

    sig_sethandler(SIGINT, sigint_handler);

    // open fifo1 in read only mode
    while (1) {

        fd1 = open(FIFO1, O_RDONLY);
        SYSCHECK(fd1, "open");

        //read data from fifo1
        //ssize_t bR = read(fd1, buffer, MAX_LEN);
        //SYSCHECK(bR, "read");
        ssize_t bR = read_fifo(fd1, buffer, MAX_LEN);
        buffer[bR] = '\0';

        //from string to int
        int n = atoi(buffer);

        msg_t **msg_map;

        msg_map = (msg_t **) calloc(n, sizeof(msg_t *));

        //costruzione messaggio da scrivere sulla shmem
        snprintf(buffer, sizeof(buffer), "\t← <Server>: Sono pronto per la ricezione di %d file\n\n", n);

        //write data on shmem
        //strcpy(shmem, buffer);
        shmem[0] = (msg_t *) malloc(sizeof(msg_t));
        //controllo malloc
        shmem[0]->message = strdup(buffer);

        //wake up client
        semOp(semid,0,SIGNAL);

        int index = 0;
        msg_t tmp;
        tmp.name = (char *) calloc(PATH_MAX,sizeof(char));
        MCHECK(tmp.name);
        tmp.message = (char *) calloc(PATH_MAX,sizeof(char));
        MCHECK(tmp.message);

        while (1){
            //polling fifo1
            if (semctl(semid_counter, MAX_SEM_FIFO1, GETVAL, arg.val) == -1)
                errExit("semctl GETVAL");

            if(arg.val != 50){
                read(fd1, &tmp, GET_MSG_SIZE(tmp));
                msg_map[tmp.client] = (msg_t *) calloc(PARTS, GET_MSG_SIZE(tmp));
                msg_map[tmp.client][0].message = strdup(tmp.message);
                msg_map[tmp.client][0].name = strdup(tmp.name);
                msg_map[tmp.client][0].type = tmp.type;
                msg_map[tmp.client][0].pid = tmp.pid;
                semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                printf("→ <Server>: ricevuto parte %d del file: %s\n", 1, tmp.name);
            }
            else {
                //nothing to read -> trunc file
            }

            //polling fifo2
            if (semctl(semid_counter, MAX_SEM_FIFO2, GETVAL, arg.val) == -1)
                errExit("semctl GETVAL");

            if(arg.val != 50){
                read(fd2, &tmp, GET_MSG_SIZE(tmp));
                msg_map[tmp.client] = (msg_t *) calloc(PARTS, GET_MSG_SIZE(tmp));
                msg_map[tmp.client][1].message = strdup(tmp.message);
                msg_map[tmp.client][1].name = strdup(tmp.name);
                msg_map[tmp.client][1].type = tmp.type;
                msg_map[tmp.client][1].pid = tmp.pid;
                semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                printf("→ <Server>: ricevuto parte %d del file: %s\n", 2, tmp.name);
            }
            else {
                //nothing to read -> trunc file
            }

            //polling msq
            semOp(semid, SEMMSQ, WAIT);
            msg_receive(msqid, &tmp, 0, IPC_NOWAIT);
            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
            msg_map[tmp.client] = (msg_t *) calloc(PARTS, GET_MSG_SIZE(tmp));
            msg_map[tmp.client][2].message = strdup(tmp.message);
            msg_map[tmp.client][2].name = strdup(tmp.name);
            msg_map[tmp.client][2].type = tmp.type;
            msg_map[tmp.client][2].pid = tmp.pid;
            semOp(semid, SEMMSQ, SIGNAL);
            printf("→ <Server>: ricevuto parte %d del file: %s\n", 3, tmp.name);

            //polling shmem
            msg_t *current = shmem[index];

            semOp(semid, SEMSHM, WAIT);
            if(there_is_message(shmem)){
                if(!is_empty(shmem, index))
                    msg_map[current->client] = (msg_t *) calloc(PARTS, GET_MSG_SIZE(tmp));
                    msg_map[current->client][3].type = current->type;
                    msg_map[current->client][3].name = strdup(current->name);
                    msg_map[current->client][3].type = current->type;
                    msg_map[current->client][3].pid = current->pid;
                    semOp(semid_counter, MAX_SEM_SHM, SIGNAL);
                printf("→ <Server>: ricevuto parte %d del file: %s\n", 4, current->name);
            }
            semOp(semid, SEMMSQ, SIGNAL);
        }

        //controllare qui le righe(processi) che hanno tutte e quattro le parti
        //for (int i = 0; i < 37; ++i) {
        //    if(prova(..., i)){
                //file
        //    }
        //}
        //per ogni processo che ha consegnato tutte 4 le parti salvarle in un file (vedi pdf -> server)
    }
    //si mette in ascolto sulla msq
}
