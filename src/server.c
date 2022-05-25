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

    // Create a semaphore semid_counter with 50 max messages
    printf("→ <Server>: Waiting semaphore counter set allocation...\n");
    semid_counter = alloc_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    // initialize semaphore semid_counter
    unsigned short values[] = { MAXMSG, MAXMSG, MAXMSG};
    union semun arg;
    arg.array = values;
    semctl(semid_counter, 0, SETALL, arg);

    // Create a semaphore semid_sync mutex/synchronization
    printf("→ <Server>: Waiting semaphore synchronization set allocation...\n");
    semid_sync = alloc_semaphore(KEYSEM_SYNC, SEMNUM_SYNC);

    // initialize semaphore semid_synch: synchronization
    arg.val = 0;
    semctl(semid_sync, SYNC_SHM, SETVAL, arg);
    semctl(semid_sync, SYNC_FIFO1, SETVAL, arg);
    semctl(semid_sync, SYNCH_MSQ, SETVAL, arg);
    // initialize semaphore semid_synch: mutex
    arg.val = 1;
    semctl(semid_sync, SEMCHILD, SETVAL, arg);
    semctl(semid_sync, SEMSHM, SETVAL, arg);

    // create message queue
    printf("→ <Server>: Waiting message queue allocation...\n");
    msqid = alloc_message_queue(KEYMSQ);

    // create FIFOs
    printf("→ <Server>: Waiting fifo1 allocation...\n");
    make_fifo(FIFO_1, 1);
    printf("→ <Server>: Waiting fifo2 allocation...\n");
    make_fifo(FIFO_2, 2);
    // fifo create flag (used to check if fifo exists)
    fifo_flag = 1;

    // set sigint signal handler
    sig_sethandler(SIGINT, sigint_handler);

    struct sembuf sop[PARTS]; // useless ???
    //size_t index = 0;// TODO da cancellare

    // open fifo1 in read only mode & non block
    fd1 = open(FIFO_1, O_RDONLY | O_NONBLOCK);
    // open fifo2 in read only mode  & non block
    fd2 = open(FIFO_2, O_RDONLY | O_NONBLOCK);

    // only for debug (number of iteration)
    int times = 1;
    while (1) {
        //waiting server write on fifo1
        printf("→ <Server>: Waiting Client response on FIFO1... \n");
        semOp(semid_sync, SYNC_FIFO1, WAIT);
        ssize_t bR = read_fifo(fd1, 1, string_buffer, MAX_LEN);
        string_buffer[bR] = '\0';
        printf("← <Client>: There are %s file to send\n", string_buffer);

        //from string to int
        size_t n = atoi(string_buffer);

        //matrice del server per la memorizzazione dei messaggi fatta sullo heap
        //msg_t** msg_map= (msg_t **) calloc(n, sizeof(msg_t *));
        //matrice fatta sulllo stack
        msg_t msg_map[37][4]; // TODO heap or stack? da sostituire row = 100
        //allocazione di 4 colonne per ogni client che conterrà la parte 1...4
        //for (size_t i = 0; i < n; i++) {
        //    msg_map[i] = (msg_t *) calloc(PARTS, sizeof(msg_t));
        //    MCHECK(msg_map[i]);
        //}

        //costruzione messaggio da scrivere sulla shmem
        snprintf(string_buffer, sizeof(string_buffer), "← <Server>: Sono pronto per la ricezione di %zu file\n\n", n);
        // scrivo sulla shmem
        strcpy(shmem->message, string_buffer);
        //sblocco il client in attesa di lettura
        semOp(semid_sync, SYNC_SHM, SIGNAL);

        //serve per salvare il valore del semaforo per la trunc
        //int val = 0;
        // buffer di appoggio per i messaggi
        msg_t msg_buffer;
        // Contiene i caratteri letti nelle fifo
        ssize_t Br = 0;
        // risultato delle chiamate alle ipc
        int res = 0;
        //sup si può usare al posto di cont
        //int sup[37] = {0};
        // serve per vedere se server ha letto tutti i file
        int cont = 0;
        // TODO controllare le iterazioni, non sono deterministiche (a volte funziona a volte va in loop)
        while (cont < (37 * 4))
        {
            // ------------------------FIFO_1-------------------------------------------
            errno = 0;
             if((Br = read(fd1, &msg_buffer, sizeof(msg_buffer))) != -1) {
                if(Br > 0){
                    // salvo il messaggio
                    //printf("Il processo %ld manda in fifo1: <%s> \n", msg_buffer.client, msg_buffer.message)
                    msg_map[msg_buffer.client][FIFO1].client = msg_buffer.client;
                    msg_map[msg_buffer.client][FIFO1].type = msg_buffer.type;
                    msg_map[msg_buffer.client][FIFO1].pid = msg_buffer.pid;
                    strcpy(msg_map[msg_buffer.client][FIFO1].message , msg_buffer.message);
                    strcpy(msg_map[msg_buffer.client][FIFO1].name , msg_buffer.name);
                    //printf("Il processo %ld riceve su fifo1: <%ld> \n", msg_buffer.client + 1, msg_map[msg_buffer.client][FIFO1].type);
                    cont++;
                    //printf("fifo1 valore : %d processo n %ld\n", (sup[msg_buffer.client]), msg_buffer.client);
                    //sblocco una posizione per la scrittura

                    //printf("→ <Server>: salvata parte %d del file: %s\n",FIFO1, msg_map[msg_buffer.client][FIFO1].name);
                    semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                }
                else{
                    //printf("→ <Server>: ho letto 0 caratteri\n");
                }
            }
             else{
                 if(errno != EAGAIN){
                     errExit("read failed: \n");
                 }
             }

            //if ((val = semctl(semid_counter, MAX_SEM_FIFO1, GETVAL, 0)) == -1)
            //    errExit("semctl GETVAL");

            // if (val == MAXMSG) {
            // trunc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?
            //posso chiudere il file descriptor e riaprirlo con trunc?
            //}
            //printf("Post fifo1\n");

            //--------------------------FIFO_2-------------------------------------------
            errno = 0;
            if((Br = read(fd2, &msg_buffer, sizeof(msg_buffer))) != -1) {
                if(Br > 0){
                    // salvo il messaggio
                    //printf("Il processo %ld manda in fifo2: <%s> \n", msg_buffer.client, msg_buffer.message);
                    msg_map[msg_buffer.client][FIFO2].client = msg_buffer.client;
                    msg_map[msg_buffer.client][FIFO2].type = msg_buffer.type;
                    msg_map[msg_buffer.client][FIFO2].pid = msg_buffer.pid;
                    strcpy(msg_map[msg_buffer.client][FIFO2].message , msg_buffer.message);
                    strcpy(msg_map[msg_buffer.client][FIFO2].name , msg_buffer.name);
                    //printf("Il processo %ld riceve su fifo2: <%ld> \n", msg_buffer.client + 1, msg_map[msg_buffer.client][FIFO2].type);
                    //sblocco una posizione per la scrittura
                    cont++;
                    //printf("fifo2 valore : %d processo n %ld\n", (sup[msg_buffer.client]), msg_buffer.client);
                    //printf("→ <Server>: salvata parte %d del file: %s\n",FIFO2 + 1, msg_map[msg_buffer.client][FIFO2].name);

                    semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                }
            }
            else{
                if(errno != EAGAIN){
                    errExit("read failed: \n");
                }
            }

            //if ((val = semctl(semid_counter, MAX_SEM_FIFO2, GETVAL, 0)) == -1) {
            //    errExit("semctl GETVAL");
            //}

            //printf("Post fifo2\n");

             //if(val == MAXMSG) {
             //trunc file attenzione un client potrebbe scrivere mentre si tronca, sezione critica?
                 //close(fd2);
            //     fd2 = open(FIFO_2, O_RDONLY | O_NONBLOCK | O_TRUNC);
            //     printf("<Server>: Ho troncato la fifo2\n");
            // }

            //--------------------------MESSAGE QUEUE----------------------------------------------
            errno = 0;
            //printf("pre msq\n");
            res = msgrcv(msqid, &msg_buffer, MSGSIZE , 0, IPC_NOWAIT);

            if (res == -1 && errno == ENOMSG) {
                //printf("→ <Server>: Non ci sono messaggi nella Message Queue\n");
            } else if (res > 0){
                //salvo il messaggio
                msg_map[msg_buffer.client][MSQ].client = msg_buffer.client;
                msg_map[msg_buffer.client][MSQ].type = msg_buffer.type;
                msg_map[msg_buffer.client][MSQ].pid = msg_buffer.pid;
                strcpy(msg_map[msg_buffer.client][MSQ].message , msg_buffer.message);
                strcpy(msg_map[msg_buffer.client][MSQ].name , msg_buffer.name);
                cont++;
                //printf("msq valore : %d processo n %ld\n", (sup[msg_buffer.client]), msg_buffer.client);
                //printf("→ <Server>: salvata parte %d del file: %s\n", MSQ + 1, msg_map[msg_buffer.client][MSQ].name);
                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
            }
            //printf("post msq\n");
            //}

            //---------------------------SHARED MEMORY-----------------------------------------

            // prova a prendere il mutex
            sop[SHM].sem_num = SEMSHM;
            sop[SHM].sem_op  = WAIT;
            sop[SHM].sem_flg = IPC_NOWAIT;

            //printf("pre shared\n");
            for (size_t id = 0; id < MAXMSG; id++) {
                if(shmem[id].type == 1){
                    if(semop(semid_sync, &sop[SHM], 1) == 0){
                        //semop()
                        //printf("Index: %zu: \n", id);
                        //printf("Sono nella shared memory sono: %d  \n", shmem[id].pid);
                        // salvo il messaggio
                        msg_map[shmem[id].client][SHM].client = shmem[id].client;
                        msg_map[shmem[id].client][SHM].pid = shmem[id].pid;
                        msg_map[shmem[id].client][SHM].type = shmem[id].type;
                        strcpy(msg_map[shmem[id].client][SHM].name , shmem[id].name);
                        strcpy(msg_map[shmem[id].client][SHM].message, shmem[id].message);
                        shmem[id].type = 0;
                        cont++;
                        //printf("shared memory valore : %d processo n %ld\n", (sup[msg_buffer.client]), msg_buffer.client);
                        // svuota il messaggio
                        //printf("→ <Server>: salvata parte %d del file: %s\n", SHM + 1, msg_map[shmem[index].client][SHM].name);
                        semOp(semid_sync, SEMSHM, SIGNAL);
                    }
                }
            }
            //printf("post shared \n");
            printf("cont : %d\n", cont);
            char msg[MAX_LEN];

            for (size_t child = 0; child < n; child++) {
                if(msg_map[child][FIFO1].type == 1 && msg_map[child][FIFO2].type == 1
                   && msg_map[child][MSQ].type == 1 && msg_map[child][SHM].type == 1) {

                    char* file_name = msg_map[child][FIFO1].name;
                    int fid = open(strcat(file_name, "_out"), O_WRONLY | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD);

                    if( fid == -1)
                        errExit("write failed frifo1\n");

                    snprintf(msg, MAX_LEN, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n %s \n",
                             1, msg_map[child][FIFO1].name, msg_map[child][FIFO1].pid, "FIFO1", msg_map[child][FIFO1].message);

                    Br = write(fid, msg, sizeof(msg));

                    if(Br == -1){
                        errExit("write failed fifo1\n");
                    }

                    snprintf(msg, MAX_LEN, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n %s \n",
                             1, msg_map[child][FIFO2].name, msg_map[child][FIFO2].pid, "FIFO2", msg_map[child][FIFO2].message);

                    Br = write(fid, msg, sizeof(msg));

                    if(Br == -1){
                        errExit("write failed fifo2\n");
                    }

                    snprintf(msg, MAX_LEN, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n %s \n",
                             1, msg_map[child][MSQ].name, msg_map[child][MSQ].pid, "MSQ",  msg_map[child][MSQ].message);

                    Br = write(fid, msg, sizeof(msg));

                    if(Br == -1){
                        errExit("write failed MSQ\n");
                    }

                    snprintf(msg, MAX_LEN, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n %s \n",
                             1, msg_map[child][SHM].name, msg_map[child][SHM].pid, "SHMEM", msg_map[child][SHM].message);

                    Br = write(fid, msg, sizeof(msg));

                    if(Br == -1){
                        errExit("write failed SHMMEM\n");
                    }

                    //imposta type a 0 in modo che l'if iniziale non sia più true
                    // per questo child dato che l'opeazione è stata completata
                    msg_map[child][FIFO1].type = 0;

                    close(fid);
                }
            }
        }
        printf("Ho finito la %d iterazione!!!!\n", times++);

        //il server prepara il messaggio
        msg_t result;
        result.type = 1;
        strcpy(result.message, "<Server>: Ho finito di leggere i file!\n");

        // invia il messaggio
        msgsnd(msqid, &result, MSGSIZE, 0);
        //sblocca il client
        semOp(semid_sync, SYNCH_MSQ, SIGNAL);

        //printf("%s", result.message);
        //printf("\n\n ho letto QUASI tutto!\n\n");
        //per ogni processo che ha consegnato tutte 4 le parti salvarle in un file (vedi pdf -> server)
    }
}
