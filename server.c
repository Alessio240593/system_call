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
pid_t client_pid = -1;
msg_t *shmem = NULL;

void sigint_handler(int sig)
{
    printf("\t→ <Server>: Received signal %s\n\n", signame[sig]);

    if (fd1 >= 0) {
        close_fd(fd1);
    }

    if (fd2 >= 0) {
        close_fd(fd2);
    }

    if (fifo_flag){
    	printf("→ <Server>: Waiting for FIFO1 removal...\n");
        remove_fifo(FIFO_1, 1);
        printf("→ <Server>: Waiting for FIFO2 removal...\n");
        remove_fifo(FIFO_2, 2);
        fifo_flag = 0;
    }
    
    if (msqid >= 0) {
    	printf("→ <Server>: Waiting for message queue n°%d removal...\n", KEYMSQ);
        remove_message_queue(msqid);
    }

    if (shmid >= 0 && shmem != NULL) {
        free_shared_memory(shmem);
        printf("→ <Server>: Waiting for shared memory n°%d removal...\n", KEYSHM);
        remove_shared_memory(shmid);
    }

    if (semid_sync >= 0)	{
    	printf("→ <Server>: Waiting for semaphore set n°%d removal...\n", KEYSEM_SYNC);
        remove_semaphore(semid_sync);
    }
    	
    if (semid_counter >= 0) {
    	printf("→ <Server>: Waiting for semaphore set n°%d removal...\n", KEYSEM_COUNTER);
        remove_semaphore(semid_counter);
    }
    	
    // uccide il client
    if (client_pid != -1) {
        if (kill(client_pid, SIGUSR1) == -1) {
            errExit("couldn't kill client: ");
        }
    }
    
    printf("→ <Server>: Exit success!\n");

    exit(EXIT_SUCCESS);
}


int main(void)
{
    // create FIFOs
    printf("→ <Server>: Waiting fifo1 allocation...\n");
    make_fifo(FIFO_1, 1);
    printf("→ <Server>: Waiting fifo2 allocation...\n");
    make_fifo(FIFO_2, 2);
    // fifo create flag (used to check if fifo exists)
    fifo_flag = 1;
    
    // create message queue
    printf("→ <Server>: Waiting message queue n°%d allocation...\n", KEYMSQ);
    msqid = alloc_message_queue(KEYMSQ);

    // create shmem
    printf("→ <Server>: Waiting shared memory n°%d allocation...\n", KEYSHM);
    shmid = alloc_shared_memory(KEYSHM,SHMSIZE);
    shmem = (msg_t *)attach_shared_memory(shmid, 0);

    // Create a semaphore semid_counter with 50 max messages
    printf("→ <Server>: Waiting semaphore set n°%d allocation...\n", KEYSEM_COUNTER);
    semid_counter = alloc_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    // initialize semaphore semid_counter
    unsigned short values[] = { MAXMSG, MAXMSG, MAXMSG};
    union semun arg;
    arg.array = values;
    semctl(semid_counter, 0, SETALL, arg);

    // Create a semaphore semid_sync mutex/synchronization
    printf("→ <Server>: Waiting semaphore set n°%d allocation...\n", KEYSEM_SYNC);
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
  
    printf("\n→ <Server>: Press <ctr+c> to terminate... \n");

    // set sigint signal handler
    sig_sethandler(SIGINT, sigint_handler);

    // open fifo1 in read only mode & non block
    fd1 = open(FIFO_1, O_RDONLY | O_NONBLOCK);
    // open fifo2 in read only mode  & non block
    fd2 = open(FIFO_2, O_RDONLY | O_NONBLOCK);

    msg_t msg_client_pid;
    if (msgrcv(msqid, &msg_client_pid, MSGSIZE, 0, 0) == -1) {
        errExit("failed to receive pid from client: ");
    }
    client_pid = msg_client_pid.pid;


    // big declaration
    char string_buffer[MAX_LEN];
    size_t n;
    struct sembuf sop[PARTS];

    while (1) {
        //waiting server write on fifo1
        printf("→ <Server>: Waiting Client response on FIFO1... \n");
        printf("\n→ <Server>: Press <ctr+c> to terminate... \n");
        semOp(semid_sync, SYNC_FIFO1, WAIT);
        ssize_t bR = read_fifo(fd1, 1, string_buffer, MAX_LEN);
        string_buffer[bR] = '\0';
        printf("\n← <Client>: There are %s file to send\n", string_buffer);

        //from string to int
        n = atoi(string_buffer);

        //matrice del server per la memorizzazione dei messaggi fatta sullo heap
        /*
        msg_t** msg_map= (msg_t **) calloc(n, sizeof(msg_t *));

        for (size_t i = 0; i < n; i++) {
            msg_map[i] = (msg_t *) calloc(PARTS, sizeof(msg_t));
            MCHECK(msg_map[i]);
        }
        */
        msg_t msg_map[100][PARTS];

        //costruzione messaggio da scrivere sulla shmem
        snprintf(string_buffer, sizeof(string_buffer), "← <Server>: Sono pronto per la ricezione di %zu file\n\n", n);
        // scrivo sulla shmem
        strcpy(shmem->message, string_buffer);
        //sblocco il client in attesa di lettura
        semOp(semid_sync, SYNC_SHM, SIGNAL);

        // buffer di appoggio per i messaggi
        msg_t msg_buffer;
        // Contiene i caratteri letti nelle fifo
        ssize_t Br = 0;
        // risultato delle chiamate alle ipc
        int res = 0;
        // serve per vedere se server ha letto tutti i file
        size_t cont = 0;

        while (cont < (n * PARTS))
        {
            // ------------------------FIFO_1-------------------------------------------
            errno = 0;
            if ((Br = read(fd1, &msg_buffer, sizeof(msg_buffer))) != -1) {
                if (Br > 0) {
                    // salvo il messaggio
                    msg_map[msg_buffer.client][FIFO1].client = msg_buffer.client;
                    msg_map[msg_buffer.client][FIFO1].type = msg_buffer.type;
                    msg_map[msg_buffer.client][FIFO1].pid = msg_buffer.pid;
                    strcpy(msg_map[msg_buffer.client][FIFO1].message , msg_buffer.message);                   
                    strcpy(msg_map[msg_buffer.client][FIFO1].name , msg_buffer.name);
                    cont++;
                    //sblocco una posizione per la scrittura
                    semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                }
            }
             else {
                if (errno != EAGAIN) {
                    errExit("read failed: \n");
                }
             }

            //--------------------------FIFO_2-------------------------------------------
            errno = 0;
            if ((Br = read(fd2, &msg_buffer, sizeof(msg_buffer))) != -1) {
                if (Br > 0) {
                    // salvo il messaggio
                    msg_map[msg_buffer.client][FIFO2].client = msg_buffer.client;
                    msg_map[msg_buffer.client][FIFO2].type = msg_buffer.type;
                    msg_map[msg_buffer.client][FIFO2].pid = msg_buffer.pid;
                    strcpy(msg_map[msg_buffer.client][FIFO2].message , msg_buffer.message);
                    strcpy(msg_map[msg_buffer.client][FIFO2].name , msg_buffer.name);
                    //sblocco una posizione per la scrittura
                    cont++;
                    semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                }
            }
            else {
                if (errno != EAGAIN) {
                    errExit("read failed: \n");
                }
            }

            //--------------------------MESSAGE QUEUE----------------------------------------------
            errno = 0;

            res = msgrcv(msqid, &msg_buffer, MSGSIZE , 0, IPC_NOWAIT);

            if (res == -1 && errno != ENOMSG) {
                errExit("msgrcv failed:");
            } else if (res > 0) {
                //salvo il messaggio
                msg_map[msg_buffer.client][MSQ].client = msg_buffer.client;
                msg_map[msg_buffer.client][MSQ].type = msg_buffer.type;
                msg_map[msg_buffer.client][MSQ].pid = msg_buffer.pid;
                strcpy(msg_map[msg_buffer.client][MSQ].message , msg_buffer.message);
                strcpy(msg_map[msg_buffer.client][MSQ].name , msg_buffer.name);
                cont++;
                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
            }

            //---------------------------SHARED MEMORY-----------------------------------------

            // prova a prendere il mutex
            sop[SHM].sem_num = SEMSHM;
            sop[SHM].sem_op  = WAIT;
            sop[SHM].sem_flg = IPC_NOWAIT;

            for (size_t id = 0; id < MAXMSG; id++) {
                if (shmem[id].type == 1) {
                    if (semop(semid_sync, &sop[SHM], 1) == 0) {
                        msg_map[shmem[id].client][SHM].client = shmem[id].client;
                        msg_map[shmem[id].client][SHM].pid = shmem[id].pid;
                        msg_map[shmem[id].client][SHM].type = shmem[id].type;
                        strcpy(msg_map[shmem[id].client][SHM].name , shmem[id].name);
                        strcpy(msg_map[shmem[id].client][SHM].message, shmem[id].message);
                        shmem[id].type = 0;
                        cont++;
                        semOp(semid_sync, SEMSHM, SIGNAL);
                    }
                }
            }
            
            //scrivo nei file out i vari messaggi
            char msg[PATH_MAX];
            char file_name[PATH_MAX];

            for (size_t child = 0; child < n; child++) {
                if (msg_map[child][FIFO1].type == 1 && msg_map[child][FIFO2].type == 1
                   && msg_map[child][MSQ].type == 1 && msg_map[child][SHM].type == 1)
                {
                    strncpy(file_name, append_out(msg_map[child][FIFO1].name), PATH_MAX);
					
                    int fid = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

                    if (fid == -1) {
                        errExit("open failed: \n");
                    }

                    snprintf(msg, PATH_MAX, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n%s\n",
                             1, msg_map[child][FIFO1].name, msg_map[child][FIFO1].pid, "FIFO1", msg_map[child][FIFO1].message);

                    Br = write(fid, &msg, strlen(msg));

                    if (Br == -1) {
                        errExit("write failed fifo1\n");
                    }

                    snprintf(msg, PATH_MAX, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n%s\n",
                             2, msg_map[child][FIFO2].name, msg_map[child][FIFO2].pid, "FIFO2", msg_map[child][FIFO2].message);

                    Br = write(fid, &msg, strlen(msg));

                    if (Br == -1) {
                        errExit("write failed fifo2\n");
                    }

                    snprintf(msg, PATH_MAX, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n%s\n",
                             3, msg_map[child][MSQ].name, msg_map[child][MSQ].pid, "MSQ",  msg_map[child][MSQ].message);

                    Br = write(fid, msg, strlen(msg));

                    if (Br == -1) {
                        errExit("write failed MSQ\n");
                    }

                    snprintf(msg, PATH_MAX, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n%s\n",
                             4, msg_map[child][SHM].name, msg_map[child][SHM].pid, "SHMEM", msg_map[child][SHM].message);

                    Br = write(fid, &msg, strlen(msg));

                    if (Br == -1) {
                        errExit("write failed SHMMEM\n");
                    }

                    close(fid);
                }
            }

        }

        //il server prepara il messaggio
        msg_t result;
        result.type = 1;
        strcpy(result.message, "← <Server>: Ho finito di leggere i file!\n\n");

        // invia il messaggio
        msgsnd(msqid, &result, MSGSIZE, 0);
        //sblocca il client
        semOp(semid_sync, SYNCH_MSQ, SIGNAL);
    }
}
