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
    shmid = alloc_shared_memory(KEYSHM,SHMSIZE);
    shmem = (msg_t *)attach_shared_memory(shmid, 0);

    //array di supporto shmem
    supporto_id = alloc_shared_memory(KEY_SUPPORT, SUPPORTO_SIZE);
    supporto = (int *) attach_shared_memory(supporto_id, 0);

    //initialize supporto
    for (size_t i = 0; i < MAXMSG; i++){
        supporto[i] = 0;
    }

    // Create a semaphore with 50 max messages
    semid_counter = alloc_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    //initialize semaphore
    unsigned short values[] = { MAXMSG, MAXMSG, MAXMSG, MAXMSG};
    union semun arg;
    arg.array = values;
    semctl(semid_counter, 0, SETALL, arg);

    //create synch server
    semid = alloc_semaphore(KEYSEM_SYNC, SEMNUM_SYNC);

    //set sem0 at 0 (semaforo per la sincronizzazione, poi diventerà un mutex)
    arg.val = 0;
    semctl(semid, SEMSHM, SETVAL, arg);

    // set sem1 and sem2 at 1 (mutex)
    arg.val = 1;
    semctl(semid, SEMMSQ, SETVAL, arg);
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
        fd1 = open_fifo(FIFO1, O_RDONLY);

        // open fifo2 in read only mode
        fd2 = open_fifo(FIFO2, O_RDONLY);

        ssize_t bR = read_fifo(fd1, string_buffer, MAX_LEN);
        string_buffer[bR] = '\0';

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
        semOp(semid,SEMSHM,SIGNAL, 0);
        //increase sem for mutex
        semOp(semid,SEMSHM,SIGNAL, 0);

        int val = 0;
        size_t index = 0;
        msg_t msg_buffer;

        while (1) {
            // ------------------------FIFO1-------------------------------------------
            read_fifo(fd1, &msg_buffer, sizeof(msg_buffer));
            //salvo il messaggio
            strcpy(msg_map[msg_buffer.client][0].message , msg_buffer.message);
            strcpy(msg_map[msg_buffer.client][0].name , msg_buffer.name);
            msg_map[msg_buffer.client][0].type = msg_buffer.type;
            msg_map[msg_buffer.client][0].pid = msg_buffer.pid;
            //sblocco una posizione per la scrittura
            semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL, 0);

            printf("→ <Server>: salvata parte %d del file: %s\n", 1, msg_map[index][0].name);

            if ((val = semctl(semid_counter, MAX_SEM_FIFO1, GETVAL, 0)) == -1)
                errExit("semctl GETVAL");

            if(val == 50 )
                //truc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?


            //--------------------------FIFO2-------------------------------------------
            read_fifo(fd2, &msg_buffer, sizeof(msg_buffer));

            //salvo il messaggio
            strcpy(msg_map[msg_buffer.client][1].message , msg_buffer.message);
            strcpy(msg_map[msg_buffer.client][1].name , msg_buffer.name);
            msg_map[msg_buffer.client][1].type = msg_buffer.type;
            msg_map[msg_buffer.client][1].pid = msg_buffer.pid;
            //sblocco una posizione per la scrittura
            semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL, 0);

            printf("→ <Server>: salvata parte %d del file: %s\n", 2, msg_map[index][1].name);

            if ((val = semctl(semid_counter, MAX_SEM_FIFO2, GETVAL, 0)) == -1)
                errExit("semctl GETVAL");

            if(val == 50 )
                //truc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?

            //--------------------------MESSAGE QUEUE----------------------------------------------
            //prova a prendere il mutex
            semOp(semid, SEMMSQ, WAIT, 0); //serve IPC_NOWAIT

            //msg_receive(msqid, &msg_buffer, 0, IPC_NOWAIT);
            errno = 0;

            // TODO controllare che msgrcv funzioni
            msgrcv(msqid, &msg_buffer, MSGSIZE , 0, IPC_NOWAIT);

            if(errno == ENOMSG)
            {
                printf("→ <Server>: Non ci sono messaggi sulla message queue\n");
            }

            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);

            semOp(semid, SEMMSQ, SIGNAL, 0);

            //salvo il messaggio
            strcpy(msg_map[msg_buffer.client][2].message , msg_buffer.message);
            strcpy(msg_map[msg_buffer.client][2].name , msg_buffer.name);
            msg_map[msg_buffer.client][2].type = msg_buffer.type;
            msg_map[msg_buffer.client][2].pid = msg_buffer.pid;

            printf("→ <Server>: salvata parte %d del file: %s\n", 3, msg_map[index][2].name);


            //---------------------------SHARED MEMORY-----------------------------------------
            // prova a prendere il mutex
            // TODO si blocca qui
            semOp(semid, SEMSHM, WAIT, 0); //serve IPCNOWAIT
            size_t l = 0;

            //controlla se esiste una locazione libera
            while (supporto[index] == 0 && l < MAXMSG) {
                index = (index + 1) % MAXMSG;
                l++;
            }

            if (l != MAXMSG) {
                // salvo il messaggio
                msg_map[shmem[index].client][3].client = shmem[index].client;
                msg_map[shmem[index].client][3].pid = shmem[index].pid;
                msg_map[shmem[index].client][3].type = shmem[index].type;
                strcpy(msg_map[msg_buffer.client][3].name , msg_buffer.name);
                strcpy(msg_map[shmem[index].client][3].name, shmem[index].message);

                // svuota il messaggio
                supporto[index] = 0;

                semOp(semid_counter, MAX_SEM_SHM, SIGNAL, 0);
            }
            printf("→ <Server>: ricevuto e salvato parte %d del file: %s\n", 4, msg_map[shmem[index].client][3].name);
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