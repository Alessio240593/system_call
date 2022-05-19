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
    char string_buffer[MAX_LEN];

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
    // set sem1 and sem2 at 1 (si potrebbe fare una SETALL)
    semctl(semid, SEMSHM, SETVAL, arg);
    semctl(semid, SEMMSQ, SETVAL, arg);
    // create semaphore to sync clients
    semctl(semid, SEMCHILD, SETVAL, arg);

    // create message queue
    msqid = alloc_message_queue(KEYMSQ);

    // create FIFOs
    make_fifo(FIFO1);
    make_fifo(FIFO2);
    //fifo create flag (used to check if fifo exists)
    fifo_flag = 1;

    sig_sethandler(SIGINT, sigint_handler);


    while (1) {
        // open fifo1 in read only mode
        fd1 = open(FIFO1, O_RDONLY);
        SYSCHECK(fd1, "open");
        // TODO open fifo 2?
        //read data from fifo1
        //ssize_t bR = read(fd1, string_buffer, MAX_LEN);
        //SYSCHECK(bR, "read");
        ssize_t bR = read_fifo(fd1, string_buffer, MAX_LEN);
        string_buffer[bR] = '\0';

        //from string to int
        size_t n = atoi(string_buffer);

        msg_t **msg_map = (msg_t **) calloc(n, sizeof(msg_t *));
        for (size_t i = 0; i < n; i++) {
            msg_map[i] = (msg_t *) calloc(PARTS, sizeof(msg_t));
            MCHECK(msg_map[i]);
        }

        //costruzione messaggio da scrivere sulla shmem
        snprintf(string_buffer, sizeof(string_buffer), "\t← <Server>: Sono pronto per la ricezione di %d file\n\n", n);

        // write data on shmem
        shmem[0] = (msg_t *) malloc(sizeof(msg_t));
        // controllo malloc
        MCHECK(shmem[0]);
        shmem[0]->message = strdup(string_buffer);
        //free shmem[0] ??

        //wake up client
        semOp(semid,0,SIGNAL);

        size_t index = 0;
        msg_t msg_buffer;
        msg_buffer.name = (char *) calloc(PATH_MAX, sizeof(char));
        MCHECK(msg_buffer.name);
        msg_buffer.message = (char *) calloc(PATH_MAX, sizeof(char));
        MCHECK(msg_buffer.message);

        while (1) {
            // FIFO1
            if (semctl(semid_counter, MAX_SEM_FIFO1, GETVAL, arg.val) == -1)
                errExit("semctl GETVAL");

            if (arg.val != 50){
                read(fd1, &msg_buffer, GET_MSG_SIZE(&msg_buffer));
                printf("→ <Server>: ricevuto parte %d del file: %s\n", 1, msg_buffer.name);

                msg_map[msg_buffer.client][0].message = strdup(msg_buffer.message);
                msg_map[msg_buffer.client][0].name = strdup(msg_buffer.name);
                msg_map[msg_buffer.client][0].type = msg_buffer.type;
                msg_map[msg_buffer.client][0].pid = msg_buffer.pid;
                semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                printf("→ <Server>: salvata parte %d del file: %s\n", 1, msg_map[index][0].name);
            }
            else {
                //nothing to read -> trunc file
            }


            // FIFO2
            if (semctl(semid_counter, MAX_SEM_FIFO2, GETVAL, arg.val) == -1)
                errExit("semctl GETVAL");

            if(arg.val != 50){
                read(fd2, &msg_buffer, GET_MSG_SIZE(&msg_buffer));
                printf("→ <Server>: ricevuto parte %d del file: %s\n", 2, msg_buffer.name);

                msg_map[msg_buffer.client] = (msg_t *) calloc(PARTS, GET_MSG_SIZE(&msg_buffer));
                msg_map[msg_buffer.client][1].message = strdup(msg_buffer.message);
                msg_map[msg_buffer.client][1].name = strdup(msg_buffer.name);
                msg_map[msg_buffer.client][1].type = msg_buffer.type;
                msg_map[msg_buffer.client][1].pid = msg_buffer.pid;
                semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                printf("→ <Server>: salvata parte %d del file: %s\n", 2, msg_map[index][1].name);
            }
            else {
                //nothing to read -> trunc file
            }


            // MESSAGE QUEUE
            semOp(semid, SEMMSQ, WAIT);
            msg_receive(msqid, &msg_buffer, 0, IPC_NOWAIT);
            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
            printf("→ <Server>: ricevuto parte %d del file: %s\n", 3, msg_buffer.name);

            msg_map[msg_buffer.client] = (msg_t *) calloc(PARTS, GET_MSG_SIZE(&msg_buffer));
            msg_map[msg_buffer.client][2].message = strdup(msg_buffer.message);
            msg_map[msg_buffer.client][2].name = strdup(msg_buffer.name);
            msg_map[msg_buffer.client][2].type = msg_buffer.type;
            msg_map[msg_buffer.client][2].pid = msg_buffer.pid;
            semOp(semid, SEMMSQ, SIGNAL);
            printf("→ <Server>: salvata parte %d del file: %s\n", 3, msg_map[index][2].name);


            // SHARED MEMORY
            semOp(semid, SEMSHM, WAIT);

            size_t l = 0;

            while (shmem[index] == NULL && l < n) {
                index = (index + 1) % n;
                l++;
            }
            if (l != n) {
                // leggi
                msg_map[shmem[index]->client][3] = *shmem[index];
                // elimina
                shmem[index] = NULL;
                semOp(semid_counter, MAX_SEM_SHM, SIGNAL);
                // salva l'indice

                // notifica la lettura

            }
            printf("→ <Server>: ricevuto e salvato parte %d del file: %s\n", 4, msg_map[index][3].name);
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
