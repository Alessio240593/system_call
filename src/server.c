/**
 * @file sender_manager.c
 * @brief Contiene l'implementazione del sender_manager.
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
int fd2 = -1;
int shmid = -1;
int msqid = -1;
int semid_sync = -1;
int semid_counter = -1;
int supporto_id = -1;
int *supporto = NULL;
msg_t *shmem = NULL;

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
        remove_fifo(FIFO_1, 1);
        remove_fifo(FIFO_2, 2);
        fifo_flag = 0;
    }

    if(shmid >= 0 && shmem != NULL) {
        free_shared_memory(shmem);
        remove_shared_memory(shmid);
    }

    if(supporto_id >= 0 && supporto != NULL) {
        free_shared_memory(supporto);
        remove_shared_memory(supporto_id);
    }

    if(semid_sync >= 0)
        remove_semaphore(semid_sync);

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
    printf("→ <Server>: Waiting shared memory allocation...\n");
    shmid = alloc_shared_memory(KEYSHM,SHMSIZE);
    shmem = (msg_t *)attach_shared_memory(shmid, 0);

    printf("→ <Server>: Waiting support shared memory allocation...\n");
    // array di supporto shmem
    supporto_id = alloc_shared_memory(KEY_SUPPORT, SUPPORTO_SIZE);
    supporto = (int *) attach_shared_memory(supporto_id, 0);

    // initialize supporto
    for (size_t i = 0; i < MAXMSG; i++){
        supporto[i] = 0;
    }
    printf("→ <Server>: Waiting semaphore counter set allocation...\n");
    // Create a semaphore with 50 max messages
    semid_counter = alloc_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    // initialize semaphore
    unsigned short values[] = { MAXMSG, MAXMSG, MAXMSG, MAXMSG};
    union semun arg;
    arg.array = values;
    semctl(semid_counter, 0, SETALL, arg);

    printf("→ <Server>: Waiting semaphore synchronization set allocation...\n");
    // create synch server
    semid_sync = alloc_semaphore(KEYSEM_SYNC, SEMNUM_SYNC);

    // set sem0 at 0 (semaforo per la sincronizzazione, poi diventerà un mutex)
    arg.val = 0;
    semctl(semid_sync, SYNC_SHM, SETVAL, arg);
    semctl(semid_sync, SYNC_FIFO1, SETVAL, arg);

    // set sem1 and sem2 at 1 (mutex)
    arg.val = 1;
    semctl(semid_sync, SEMMSQ, SETVAL, arg);
    semctl(semid_sync, SEMCHILD, SETVAL, arg);
    semctl(semid_sync, SEMSHM, SETVAL, arg);

    printf("→ <Server>: Waiting message queue allocation...\n");
    // create message queue
    msqid = alloc_message_queue(KEYMSQ);

    printf("→ <Server>: Waiting fifo1 allocation...\n");
    // create FIFOs
    make_fifo(FIFO_1, 1);
    printf("→ <Server>: Waiting fifo2 allocation...\n");
    make_fifo(FIFO_2, 2);
    // fifo create flag (used to check if fifo exists)
    fifo_flag = 1;

    sig_sethandler(SIGINT, sigint_handler);

    struct sembuf sop[PARTS];
    size_t index = 0;

    while (1) {
        // open fifo1 in read only mode
        fd1 = open(FIFO_1, O_RDONLY);

        // open fifo2 in read only mode
        fd2 = open(FIFO_2, O_RDONLY | O_NONBLOCK);

        ssize_t bR = read_fifo(fd1, 1, string_buffer, MAX_LEN);
        string_buffer[bR] = '\0';

        close_fd(fd1);

        fd1 = open(FIFO_1, O_RDONLY | O_NONBLOCK);

        //make reading on fifo non block
        //fcntl(fd1, F_SETFL, O_NONBLOCK);
        //fcntl(fd2, F_SETFL, O_NONBLOCK);

        //from string to int
        size_t n = atoi(string_buffer);

        //matrice del server per la memorizzazione dei messaggi
        msg_t **msg_map = (msg_t **) calloc(n, sizeof(msg_t *));

        //allocazione di 4 colonne per ogni client che conterrà la parte 1...4
        for (size_t i = 0; i < n; i++) {
            msg_map[i] = (msg_t *) calloc(PARTS, sizeof(msg_t));
            MCHECK(msg_map[i]);
        }

        //costruzione messaggio da scrivere sulla shmem
        snprintf(string_buffer, sizeof(string_buffer), "← <Server>: Sono pronto per la ricezione di %zu file\n\n", n);

        // scrivo sulla shmem
        strcpy(shmem->message, string_buffer);

        //wake up client
        semOp(semid_sync, SYNC_SHM, SIGNAL, 0);

        int val = 0;
        msg_t msg_buffer;
        ssize_t Br = 0;
        int res = 0;

        while (finish(msg_map, n))
        {
            // ------------------------FIFO_1-------------------------------------------
            // TODO problema sulle fifo
            printf("→ <Server>: Sto per leggere su FIFO_1!\n");

            errno = 0;
            Br = read(fd1, &msg_buffer, sizeof(msg_buffer));
            printf("→ <Server>: Sono uscito dalla FIFO_1!\n");

            if (Br == -1 && errno == EAGAIN) {
                printf("→ <Server>: FIFO_1 vuota!\n");
            }
            else if(Br == -1) {
                errExit("read failed: ");
            }
            else {
                // salvo il messaggio
                strcpy(msg_map[msg_buffer.client][FIFO1].message , msg_buffer.message);
                strcpy(msg_map[msg_buffer.client][FIFO1].name , msg_buffer.name);
                msg_map[msg_buffer.client][FIFO1].type = msg_buffer.type;
                msg_map[msg_buffer.client][FIFO1].pid = msg_buffer.pid;
                //sblocco una posizione per la scrittura

                printf("→ <Server>: salvata parte %d del file: %s\n",
                       FIFO1, msg_map[msg_buffer.client][FIFO1].name);


            }

            semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL, 0); // TODO bloccano tutto

            if ((val = semctl(semid_counter, MAX_SEM_FIFO1, GETVAL, 0)) == -1)
                errExit("semctl GETVAL");

            // if (val == MAXMSG) {
                // trunc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?
            //}


            //--------------------------FIFO_2-------------------------------------------
            // TODO problema sulle fifo
                printf("→ <Server>: Sto per leggere su FIFO_2!\n");

            errno = 0;
            Br = read(fd2, &msg_buffer, sizeof(msg_buffer));
            printf("→ <Server>: Sono uscito dalla fifo2!\n");
            if (Br == -1 && errno == EAGAIN) {
                printf("→ <Server>: fifo2 vuota!\n");
            }
            else if(Br == -1) {
                errExit("read failed: ");
            }
            else {

                // salvo il messaggio
                strcpy(msg_map[msg_buffer.client][FIFO2].message , msg_buffer.message);
                strcpy(msg_map[msg_buffer.client][FIFO2].name , msg_buffer.name);
                msg_map[msg_buffer.client][FIFO2].type = msg_buffer.type;
                msg_map[msg_buffer.client][FIFO2].pid = msg_buffer.pid;
                //sblocco una posizione per la scrittura

                printf("→ <Server>: salvata parte %d del file: %s\n",
                       FIFO2 + 1, msg_map[msg_buffer.client][FIFO2].name);


            }

            semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL, 0); // TODO bloccano tutto

            if ((val = semctl(semid_counter, MAX_SEM_FIFO2, GETVAL, 0)) == -1) {
                errExit("semctl GETVAL");
            }

            // if(val == MAXMSG) {
                // trunc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?

            // }



            //--------------------------MESSAGE QUEUE----------------------------------------------
            //prova a prendere il mutex
            printf("→ <Server>: sto per prendere il mutex MSQ!\n");
            errno = 0;

//            sop[MSQ].sem_num = SEMMSQ;
//            sop[MSQ].sem_op  = WAIT;
//            sop[MSQ].sem_flg = IPC_NOWAIT;

//            res = semop(semid_sync, &sop[MSQ], 1);

            struct sembuf sopi = {.sem_num = SEMMSQ, .sem_op = WAIT, .sem_flg = IPC_NOWAIT};

            res = semop(semid_sync, &sopi, 1);


            if (res == -1 && errno == EAGAIN) {
                perror("→ <Server>: Message Queue occupata!: ");
                //printf("→ <Server>: Message queue occupata!\n");
            }
            else if (res == -1){
                errExit("semop failed: ");
            } else {
                errno = 0;
                // TODO controllare if(res == -1 && errno == ENOMSG) poi else if (res == -1)- error
                res = msgrcv(msqid, &msg_buffer, MSGSIZE , 0, IPC_NOWAIT);

                if (res == -1 && errno == ENOMSG) {
                    printf("→ <Server>: Non ci sono messaggi nella Message Queue\n");
                } else {

                    //salvo il messaggio
                    strcpy(msg_map[msg_buffer.client][MSQ].message , msg_buffer.message);
                    strcpy(msg_map[msg_buffer.client][MSQ].name , msg_buffer.name);
                    msg_map[msg_buffer.client][MSQ].type = msg_buffer.type;
                    msg_map[msg_buffer.client][MSQ].pid = msg_buffer.pid;

                    printf("→ <Server>: salvata parte %d del file: %s\n", MSQ + 1, msg_map[msg_buffer.client][MSQ].name);


                }

                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);

                semOp(semid_sync, SEMMSQ, SIGNAL, 0);
                printf("→ <Server>: Ho liberato il mutex MSQ!\n");
            }

            //---------------------------SHARED MEMORY-----------------------------------------

            index = 0;

            //controlla se c'è un messaggio da leggere
            while (index < MAXMSG && supporto[index] == 0) {
                //index = (index + 1) % MAXMSG;
                index++;
            }

            if (index != MAXMSG) {
                // prova a prendere il mutex
                printf("→ <Server>: Sto per prendere il mutex shmem!\n");

                sop[SHM].sem_num = SEMSHM;
                sop[SHM].sem_op  = WAIT;
                sop[SHM].sem_flg = IPC_NOWAIT;

                errno = 0;

                res = semop(semid_sync, &sop[SHM], 1);

                if (res == -1 && errno == EAGAIN)
                {
                    printf("→ <Server>: Shared memory occupata!\n");
                }
                else if(res == -1){
                    errExit("semop failed: ");
                } else{
                    // salvo il messaggio
                    msg_map[shmem[index].client][SHM].client = shmem[index].client;
                    msg_map[shmem[index].client][SHM].pid = shmem[index].pid;
                    msg_map[shmem[index].client][SHM].type = shmem[index].type;
                    strcpy(msg_map[shmem[index].client][SHM].name , shmem[index].name);
                    strcpy(msg_map[shmem[index].client][SHM].message, shmem[index].message);

                    // svuota il messaggio
                    supporto[index] = 0;
                    printf("→ <Server>: salvata parte %d del file: %s\n", SHM + 1, msg_map[shmem[index].client][SHM].name);

                    semOp(semid_sync, SEMSHM, SIGNAL, 0);
                    printf("→ <Server>: Ho liberato il mutex shmem!\n");
                }
            }
            else {
                printf("→ <Server>: Shared memory vuota\n");
            }

            // c'è qualche client bloccato?
            //if (semctl(semid_sync, SYNC_SHM, GETZCNT, 0) > 0)
            //    semOp(semid_sync, SYNC_SHM, SIGNAL, 0); // ne libera solo 1?

            //sleep(1);
            //semOp(semid_counter, MAX_SEM_SHM, SIGNAL, 0); // TODO secondo me non serve se abbiamo supporto
        }
        //per ogni processo che ha consegnato tutte 4 le parti salvarle in un file (vedi pdf -> server)
    }
    //si mette in ascolto sulla msq
}
