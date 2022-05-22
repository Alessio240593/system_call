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
int fd2 = -1;
int shmid = -1;
int msqid = -1;
int semid = -1;
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
        remove_fifo(FIFO1, 1);
        remove_fifo(FIFO2, 2);
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
    printf("→ <Server>: Waiting shared memory allocation...\n");
    shmid = alloc_shared_memory(KEYSHM,SHMSIZE);
    shmem = (msg_t *)attach_shared_memory(shmid, 0);

    printf("→ <Server>: Waiting support shared memory allocation...\n");
    //array di supporto shmem
    supporto_id = alloc_shared_memory(KEY_SUPPORT, SUPPORTO_SIZE);
    supporto = (int *) attach_shared_memory(supporto_id, 0);

    //initialize supporto
    for (size_t i = 0; i < MAXMSG; i++){
        supporto[i] = 0;
    }
    printf("→ <Server>: Waiting semaphore counter set allocation...\n");
    // Create a semaphore with 50 max messages
    semid_counter = alloc_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    //initialize semaphore
    unsigned short values[] = { MAXMSG, MAXMSG, MAXMSG, MAXMSG};
    union semun arg;
    arg.array = values;
    semctl(semid_counter, 0, SETALL, arg);

    printf("→ <Server>: Waiting semaphore synchronization set allocation...\n");
    //create synch server
    semid = alloc_semaphore(KEYSEM_SYNC, SEMNUM_SYNC);

    //set sem0 at 0 (semaforo per la sincronizzazione, poi diventerà un mutex)
    arg.val = 0;
    semctl(semid, SYNC_SHM, SETVAL, arg);
    semctl(semid, SYNC_FIFO1, SETVAL, arg);

    // set sem1 and sem2 at 1 (mutex)
    arg.val = 1;
    semctl(semid, SEMMSQ, SETVAL, arg);
    semctl(semid, SEMCHILD, SETVAL, arg);
    semctl(semid, SEMSHM, SETVAL, arg);

    printf("→ <Server>: Waiting message queue allocation...\n");
    // create message queue
    msqid = alloc_message_queue(KEYMSQ);

    printf("→ <Server>: Waiting fifo1 allocation...\n");
    // create FIFOs
    make_fifo(FIFO1, 1);
    printf("→ <Server>: Waiting fifo2 allocation...\n");
    make_fifo(FIFO2, 2);
    //fifo create flag (used to check if fifo exists)
    fifo_flag = 1;

    sig_sethandler(SIGINT, sigint_handler);

    while (1) {
        // open fifo1 in read only mode
        fd1 = open(FIFO1, O_RDONLY);

        // open fifo2 in read only mode
        fd2 = open(FIFO2, O_RDONLY | O_NONBLOCK);

        ssize_t bR = read_fifo(fd1, 1, string_buffer, MAX_LEN);
        string_buffer[bR] = '\0';

        close_fd(fd1);

        fd1 = open(FIFO1, O_RDONLY | O_NONBLOCK);

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
        semOp(semid,SYNC_SHM,SIGNAL, 0);

        int val = 0;
        msg_t msg_buffer;

        while (finish(msg_map, n)) {
            // ------------------------FIFO1-------------------------------------------
            // TODO problema sulle fifo
            printf("Sto per leggere su fifo1!\n");

            errno = 0;
            ssize_t Br = read(fd1, &msg_buffer, sizeof(msg_buffer));
            printf("Sono uscito dalla fifo1!\n");
            if (Br == -1 && errno == EAGAIN) {
                printf("→ <Server>: fifo1 vuota!\n");
            }
            else if(Br == -1) {
                errExit("read failed: ");
            }
            else{
                //semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL, 0); // TODO bloccano tutto
                //salvo il messaggio
                strcpy(msg_map[msg_buffer.client][0].message , msg_buffer.message);
                strcpy(msg_map[msg_buffer.client][0].name , msg_buffer.name);
                msg_map[msg_buffer.client][0].type = msg_buffer.type;
                msg_map[msg_buffer.client][0].pid = msg_buffer.pid;
                //sblocco una posizione per la scrittura

                printf("→ <Server>: salvata parte %d del file: %s\n", 1, msg_map[msg_buffer.client][0].name);
            }

            if ((val = semctl(semid_counter, MAX_SEM_FIFO1, GETVAL, 0)) == -1)
                errExit("semctl GETVAL");

            if(val == 50 )
                //truc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?


            //--------------------------FIFO2-------------------------------------------
            // TODO problema sulle fifo
                printf("Sto per leggere su fifo2!\n");

            errno = 0;
            Br = read(fd2, &msg_buffer, sizeof(msg_buffer));
            printf("Sono uscito dalla fifo2!\n");
            if (Br == -1 && errno == EAGAIN) {
                printf("→ <Server>: fifo2 vuota!\n");
            }
            else if(Br == -1) {
                errExit("read failed: ");
            }
            else{
                //semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL, 0); // TODO bloccano tutto
                //salvo il messaggio
                strcpy(msg_map[msg_buffer.client][1].message , msg_buffer.message);
                strcpy(msg_map[msg_buffer.client][1].name , msg_buffer.name);
                msg_map[msg_buffer.client][1].type = msg_buffer.type;
                msg_map[msg_buffer.client][1].pid = msg_buffer.pid;
                //sblocco una posizione per la scrittura

                printf("→ <Server>: salvata parte %d del file: %s\n", 2, msg_map[msg_buffer.client][1].name);
            }

            if ((val = semctl(semid_counter, MAX_SEM_FIFO2, GETVAL, 0)) == -1)
                errExit("semctl GETVAL");

            if(val == 50 )
                //truc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?



            //--------------------------MESSAGE QUEUE----------------------------------------------
            //prova a prendere il mutex
                printf("sto per prendere il mutex msq!\n");
            errno = 0;

            struct sembuf sop = {.sem_num = SEMMSQ, .sem_op = WAIT, .sem_flg = IPC_NOWAIT};

            int res = semop(semid, &sop, 1);

            if(res == -1 && errno == EAGAIN) {
                printf("→ <Server>: Message queue occupata!\n");
            }
            else if(res == -1){
                errExit("semop failed: ");
            } else{
                errno = 0;
                // TODO controllare if(res == -1 && errno == ENOMSG) poi else if (res == -1)- error
                int res = msgrcv(msqid, &msg_buffer, MSGSIZE , 0, IPC_NOWAIT);

                if(res == -1 && errno == ENOMSG) {
                    printf("→ <Server>: Non ci sono messaggi nella message queue\n");
                }else {
                    //salvo il messaggio
                    strcpy(msg_map[msg_buffer.client][2].message , msg_buffer.message);
                    strcpy(msg_map[msg_buffer.client][2].name , msg_buffer.name);
                    msg_map[msg_buffer.client][2].type = msg_buffer.type;
                    msg_map[msg_buffer.client][2].pid = msg_buffer.pid;

                    printf("→ <Server>: salvata parte %d del file: %s\n", 3, msg_map[msg_buffer.client][2].name);
                }

                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);

                semOp(semid, SEMMSQ, SIGNAL, 0);
                printf("Ho liberato il mutex msq!\n");
            }

            //---------------------------SHARED MEMORY-----------------------------------------

            size_t l = 0;
            size_t index = 0;

            //controlla se c'è un messaggio da leggere
            while (index < MAXMSG && supporto[index] == 0) {
                //index = (index + 1) % MAXMSG;
                //l++;
                index++;
            }

            if (index != MAXMSG) {
                // prova a prendere il mutex
                printf("sto per prendere il mutex shmem!\n");

                struct sembuf sop = {.sem_num = SEMSHM, .sem_op = WAIT, .sem_flg = IPC_NOWAIT};

                int res = semop(semid, &sop, 1);

                if(res == -1 && errno == EAGAIN)
                {
                    printf("→ <Server>: Shared memory occupata!\n");
                }
                else if(res == -1){
                    errExit("semop failed: ");
                } else{
                    // salvo il messaggio
                    msg_map[shmem[index].client][3].client = shmem[index].client;
                    msg_map[shmem[index].client][3].pid = shmem[index].pid;
                    msg_map[shmem[index].client][3].type = shmem[index].type;
                    strcpy(msg_map[shmem[index].client][3].name , shmem[index].name);
                    strcpy(msg_map[shmem[index].client][3].message, shmem[index].message);

                    // svuota il messaggio
                    supporto[index] = 0;
                    printf("→ <Server>: salvata parte %d del file: %s\n", 4, msg_map[shmem[index].client][3].name);

                    semOp(semid, SEMSHM, SIGNAL, 0);
                    printf("Ho liberato il mutex shmem!\n");
                }
            }
            else {
                printf("→ <Server>: Shared memory vuota\n");
            }
            if(semctl(semid, SYNC_SHM, GETZCNT, 0) > 0)
                semOp(semid, SYNC_SHM, SIGNAL, 0); // ne libera solo 1?
            sleep(1);
            //semOp(semid_counter, MAX_SEM_SHM, SIGNAL, 0); // TODO secondo me non serve se abbiamo supporto
        }
        //per ogni processo che ha consegnato tutte 4 le parti salvarle in un file (vedi pdf -> server)
    }
    //si mette in ascolto sulla msq
}