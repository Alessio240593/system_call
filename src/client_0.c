/**
 * @file client.c
 * @brief Contiene l'implementazione del client.
 */

#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "message_queue.h"
#include "_signal.h"
#include "defines.h"
#include "err_exit.h"


extern const char *signame[];
const char *path;

void sigusr1_handler(int sig)
{
    printf("\t→ <Client-0>: Received signal %s\n", signame[sig]);
    exit(EXIT_SUCCESS);
}

void sigint_handler(int sig)
{
    //notify signal
    printf("\t→ <Client-0>: Received signal %s\n", signame[sig]);
}

int main(int argc, char * argv[])
{
    //check input
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PATH>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strlen(argv[1]) <= 0) {
        fprintf(stderr, "Path too short!\n");
        exit(EXIT_FAILURE);
    }
    else if (strlen(argv[1]) >= PATH_MAX) {
        fprintf(stderr, "Path too long!\n");
        exit(EXIT_FAILURE);
    }

    if (!is_dir(argv[1])) {
        fprintf(stderr, "%s is not a directory\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    //set path
    path = argv[1];

    //create an empty sigset
    sigset_t mySet;
    sigset_t oldSet;

    //fill sigset
    sig_fillset(&mySet);

    //delete SIGINT and SIGUSR1 from the set
    sig_remove(&mySet, 2, SIGINT, SIGUSR1);

    //set mask
    printf("→ <Client-0>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
    sig_setmask(SIG_SETMASK, &mySet, NULL);
    //set signal handler
    sig_sethandler(SIGINT, sigint_handler);
    sig_sethandler(SIGUSR1, sigusr1_handler);

    //wait for a signal
    //pause();

    /* ################################################################################### */

    char buffer[LEN_INT];

    //open FIFO_1 in write only mode
    int fifo1_fd = open_fifo(FIFO_1, O_WRONLY, 0);
    SYSCHECK(fifo1_fd, "open1: ");

    //get server shmem
    printf("→ <Client-0>: Waiting shared memory synchronization...\n");
    int shmid = get_shared_memory(KEYSHM, SHMSIZE);
    msg_t *shmem = (msg_t *)attach_shared_memory(shmid, 0);

    //get supporto
    printf("→ <Client-0>: Waiting support shared memory synchronization...\n");
    int support_id = get_shared_memory(KEY_SUPPORT, SUPPORTO_SIZE);
    int *supporto = (int *)attach_shared_memory(support_id, 0);

    // get message queue
    printf("→ <Client-0>: Waiting message queue synchronization...\n");
    int msqid = get_message_queue(KEYMSQ);

    //get server semset
    printf("→ <Client-0>: Waiting semaphore set synchronization...\n");
    int semid_sync = get_semaphore(KEYSEM_SYNC , SEMNUM_SYNC);

    //get sem count
    printf("→ <Client-0>: Waiting semaphore counter set synchronization...\n");
    int semid_counter = get_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    // fill sigset
    sig_fillset(&mySet);

    // set signal mask
    printf("→ <Client-0>: Setting new mask UNBLOCK<NONE>...\n");
    sig_setmask(SIG_SETMASK, &mySet, &oldSet);

    //change working directory
    Chdir(path);

    printf("→ <Client-0>: Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), getenv("PWD"));

    /* dirlist declaration and initialization */
    dirlist_t *dir_list = (dirlist_t *) malloc(sizeof(dirlist_t));
    MCHECK(dir_list);

    dir_list->index = 0;
    dir_list->size  = 1;
    dir_list->list = (char **) calloc(dir_list->size, sizeof(char *));
    MCHECK(dir_list->list);

    init_dirlist(dir_list, path);

#ifndef _DEBUG
    dump_dirlist(dir_list, "before.txt");

    printf("child (%zu) == size (%zu) ? %s\n", dir_list->index, dir_list->size,
           dir_list->index == dir_list->size ? "sì" : "no");
#endif

    snprintf(buffer, LEN_INT, "%zu", dir_list->size);

    // open FIFO_2 in write only mode
    int fifo2_fd = open(FIFO_2, O_WRONLY | O_NONBLOCK);
    SYSCHECK(fifo2_fd, "open2: ");

    // write on fifo
    write_fifo(fifo1_fd, buffer, LEN_INT);

    close(fifo1_fd);

    // re-open FIFO_1 in write only mode and non-blocking
    fifo1_fd = open(FIFO_1, O_WRONLY | O_NONBLOCK);
    SYSCHECK(fifo1_fd, "open1: ");

    // waiting data
    semOp(semid_sync, SYNC_SHM, WAIT, 0);

    // print shmem data
    printf("%s", shmem->message);

    // start creation
    size_t child;
    pid_t pid;

    printf("\n\nSono parent: Shmid: %d, msqid: %d fifo1: %d, fifo2: %d\n\n", shmid, msqid, fifo1_fd, fifo2_fd);

    // matrice del server per la memorizzazione dei messaggi
    int **matrix_msg = (int **) calloc(dir_list->size, sizeof(int *));

    //allocazione di 4 colonne per ogni client che conterrà la parte 1...4
    for (size_t i = 0; i < dir_list->size; i++) {
        matrix_msg[i] = (int *) calloc(PARTS, sizeof(int));
        MCHECK(matrix_msg[i]);
    }

    char **parts = (char **) calloc(PARTS, sizeof(char *));
    MCHECK(parts);

    for (child = 0; child < dir_list->index; child++)
    {
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "\t→ <Client-%zu> Not created", child + 1);
            errExit("fork failed: ");
        }
        else if (pid == 0) {
            pid_t proc_pid;
            ssize_t tot_char;
            int waiting;
            size_t indx;
            // to store result
            int res = 0;
            // to store byte read
            ssize_t Br = 0;
            struct sembuf sop[PARTS]; // counter to MAX
            struct sembuf msq_sync;
            struct sembuf shm_sync;
            msg_t msgs[PARTS];

            printf("\n\nshmid: %d, msgid: %d\n\n", shmid, msqid);

            // child-th child
            int sendme_fd = open(dir_list->list[child], O_RDONLY);
            SYSCHECK(sendme_fd, "opensendme");

            tot_char = count_char(sendme_fd);
            split_file(parts, sendme_fd, tot_char);

            waiting = semctl(semid_sync, SEMCHILD, GETZCNT, 0);
            if (waiting == -1) {
                errExit("semctl failed: ");
            } else if (waiting == (int) (dir_list->index - 1)) {
                // Figlio prediletto forse è -1 perchè conta child file in attesa
                semOp(semid_sync, SEMCHILD, WAIT, 0);
            } else {
                // Figli diseredati
                semOp(semid_sync, SEMCHILD, SYNC, 0);
            }

            // start sending messages
            proc_pid = getpid();

            while(child_finish(matrix_msg , child))
            {
                //-----------------------FIFO_1-----------------------------
                if(matrix_msg[child][FIFO1] != 1){
                    // semaforo per tenere traccia delle scritture sulla FIFO_1
                    sop[FIFO1].sem_num = MAX_SEM_FIFO1;
                    sop[FIFO1].sem_op = WAIT;
                    sop[FIFO1].sem_flg = IPC_NOWAIT;

                    errno = 0;

                    res = semop(semid_counter, &sop[FIFO1], 1);

                    if (res == -1 && errno == EAGAIN) {
                        printf("\t→ <Client-%zu>: FIFO_1 non disponibile, %s\n", child + 1, strerror(errno));
                    }
                    else if (res == -1){
                        printf("sono morto e non lo sapevo");
                        errExit("semop failed: ");
                    } else {
                        //preparo il messaggio da mandare su fifo1
                        msg_t FIFO1_msg;
                        FIFO1_msg.type = 1;
                        FIFO1_msg.client = child;
                        FIFO1_msg.pid = proc_pid;
                        strcpy(FIFO1_msg.name, (dir_list->list[child]));
                        strcpy(FIFO1_msg.message, parts[FIFO1]);
                        /*
                        msgs[FIFO1].type = 1;
                        msgs[FIFO1].client = child;
                        msgs[FIFO1].pid = proc_pid;
                        strcpy(msgs[FIFO1].name, (dir_list->list[child]));
                        strcpy(msgs[FIFO1].message, parts[FIFO1]);


                        printf("→ <Client-%zu>: sono bloccato in scrittura su FIFO_1!\n", child + 1);
                        Br = write_fifo(fifo1_fd, &msgs[FIFO1], sizeof(msgs[FIFO1]));
                        */
                        errno = 0;

                        Br = write(fifo1_fd, &FIFO1_msg, sizeof(FIFO1_msg));



                        if (Br == -1 && errno == EAGAIN) {
                            printf("\t→ <Client-%zu>: FIFO_1 piena!\n", child + 1);
                            semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL, 0);
                        }
                        else if (Br == -1) {
                            semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL, 0);
                            printf("sono morto e non lo sapevo");
                            errExit("write failed: ");
                        } else {
                            if(Br != 0) {
                                matrix_msg[child][FIFO1] = 1;
                                printf("\t→ <Client-%zu>: Ho inviato il primo pezzo del file <%s> sulla FIFO_1\n",
                                       child + 1, dir_list->list[child]);
                            } else{
                                semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL, 0);
                            }
                        }
                    }
                }

                //-----------------------FIFO_2-----------------------------
                if(matrix_msg[child][FIFO2] != 1){
                    // semaforo per tenere traccia delle scritture sulla FIFO_2
                    sop[FIFO2].sem_num = MAX_SEM_FIFO2;
                    sop[FIFO2].sem_op = WAIT;
                    sop[FIFO2].sem_flg = IPC_NOWAIT;

                    errno = 0;

                    res = semop(semid_counter, &sop[FIFO2], 1);

                    if(res == -1 && errno == EAGAIN) {
                        printf("\t→ <Client-%zu>: FIFO_2 non disponibile, %s\n", child + 1, strerror(errno));
                    }
                    else if(res == -1){
                        printf("sono morto e non lo sapevo");
                        errExit("semop failed: ");
                    } else {

                        //preparo il messaggio da mandare su FIFO2
                        msg_t FIFO2_msg;
                        FIFO2_msg.type = 1;
                        FIFO2_msg.client = child;
                        FIFO2_msg.pid = proc_pid;
                        strcpy(FIFO2_msg.name, (dir_list->list[child]));
                        strcpy(FIFO2_msg.message, parts[FIFO2]);
                        /*
                        msgs[FIFO1].type = 1;
                        msgs[FIFO1].client = child;
                        msgs[FIFO1].pid = proc_pid;
                        strcpy(msgs[FIFO1].name, (dir_list->list[child]));
                        strcpy(msgs[FIFO1].message, parts[FIFO1]);


                        printf("→ <Client-%zu>: sono bloccato in scrittura su FIFO_1!\n", child + 1);
                        Br = write_fifo(fifo1_fd, &msgs[FIFO1], sizeof(msgs[FIFO1]));
                        */

                        errno = 0;

                        printf("\t→ <Client-%zu>: sono bloccato in scrittura su FIFO_2!\n", child + 1);
                        Br = write(fifo2_fd, &FIFO2_msg, sizeof(FIFO2_msg));

                        if (Br == -1 && errno == EAGAIN) {
                            printf("\t→ <Client-%zu>: FIFO_2 piena!\n", child + 1);
                            semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL, 0);
                        }
                        else if (Br == -1) {
                            semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL, 0);
                            printf("sono morto e non lo sapevo");
                            errExit("write failed: ");
                        } else {
                            if(Br != 0){
                                matrix_msg[child][FIFO2] = 1;
                                printf("\t→ <Client-%zu>: Ho inviato il secondo pezzo del file <%s> sulla FIFO_2\n",
                                       child + 1, dir_list->list[child]);
                            }else{
                                semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL, 0);
                            }
                        }
                    }
                }



                //-----------------------MESSAGE QUEUE-----------------------------
                if(matrix_msg[child][MSQ] != 1){
                    // semaforo per tenere traccia delle scritture sulla MSQ
                    sop[MSQ].sem_num = MAX_SEM_MSQ;
                    sop[MSQ].sem_op  = WAIT;
                    sop[MSQ].sem_flg = IPC_NOWAIT;

                    errno = 0;

                    res = semop(semid_counter, &sop[MSQ], 1);

                    if (res == -1 && errno == EAGAIN) {
                        printf("\t→ <Client-%zu>: Message Queue non disponibile [%d], %s\n",
                               child + 1, errno, strerror(errno));
                    }
                    else if(res == -1){
                        printf("sono morto e non lo sapevo");
                        errExit("semop failed: ");
                    } else {
                        /*
                            // inizio sezione critica
                            printf("\t→ <Client-%zu>: Sto per prendere il mutex msq!\n", child + 1);
                            // TODO provare a togliere il mutex
                            msq_sync.sem_num = SEMMSQ;
                            msq_sync.sem_op  = WAIT;
                            msq_sync.sem_flg = IPC_NOWAIT;

                            errno = 0;

                            res = semop(semid_sync, &msq_sync, 1);

                            if(res == -1 && errno == EAGAIN)
                            {
                                printf("\t→ <Client-%zu>: Message Queue mutex occupato\n", child + 1);
                                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);
                            }
                            else if(res == -1){
                                printf("sono morto e non lo sapevo");
                                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);
                                errExit("semop failed: ");
                            } else {
                            */
                        // preparo il messaggio da mandare su MSQ
                        msgs[MSQ].type = 1;
                        msgs[MSQ].client = child;
                        msgs[MSQ].pid = proc_pid;
                        strcpy(msgs[MSQ].name, (dir_list->list[child]));
                        strcpy(msgs[MSQ].message, parts[MSQ]);

                        errno = 0;

                        res = msgsnd(msqid, &msgs[MSQ], MSGSIZE, IPC_NOWAIT);

                        if (res == -1 && errno == EAGAIN) {
                            printf("\t→ <Client-%zu>: Message queue piena!\n", child + 1);
                            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);
                            semOp(semid_sync, SEMMSQ, SIGNAL, 0);
                        }
                        else if(res == -1){
                            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL, 0);
                            semOp(semid_sync, SEMMSQ, SIGNAL, 0);
                            printf("sono morto e non lo sapevo");
                            errExit("msgsnd failed: ");
                        } else {
                            matrix_msg[child][MSQ] = 1;

                            semOp(semid_sync, SEMMSQ, SIGNAL, 0);
                            printf("\t→ <Client-%zu>: Sono uscito dal mutex di msq!\n", child + 1);

                            printf("\t→ <Client-%zu>: Ho inviato il terzo pezzo del file <%s> su Message Queue\n",
                                   child + 1, dir_list->list[child]);
                        }
                    }
                }
                //}



                //-----------------------SHARED MEMORY-----------------------------
                // TODO
                // cambiare shm_msg con msgs[SHM]

                /*
                 * TODO secondo me non serve se abbiamo supporto
                 *
                 * semaforo per tenere traccia delle scritture sulla Shared Memory
                 *
                 * sop[SHM].sem_num = MAX_SEM_SHM;
                 * sop[SHM].sem_op = WAIT;
                 * sop[SHM].sem_flg = IPC_NOWAIT;
                 *
                 * semop fatta bene
                 *
                 * ...
                */
                if(matrix_msg[child][SHM] != 1){

                    shm_sync.sem_num = SEMSHM;
                    shm_sync.sem_op = WAIT;
                    shm_sync.sem_flg = IPC_NOWAIT;

                    errno = 0;

                    // inizio sezione critica
                    printf("\t→ <Client-%zu>: sto per prendere il mutex Shared Memory!\n", child);

                    res = semop(semid_sync, &shm_sync, 1);

                    if (res == -1 && errno == EAGAIN) {
                        printf("\t→ <Client-%zu>: Shared memory occupata! [%d]: %s\n", child, errno, strerror(errno));
                    }
                    else if (res == -1) {
                        printf("sono morto e non lo sapevo");
                        semOp(semid_sync, SEMSHM, SIGNAL, 0);
                        errExit("semop failed: ");
                    }
                    else {
                        indx = 0;

                        while (indx < MAXMSG && supporto[indx] == 1) {
                            indx++;
                        }

                        // se non c'è spazio il client child aspetta
                        if (indx < MAXMSG) {
                            // preparo il messaggio da inviare sulla Shared Memory
                            msg_t *shm_msg = (msg_t *) malloc(sizeof(msg_t));
                            MCHECK(shm_msg);

                            // scrivo sula shared memory
                            shmem[indx].type = 1;
                            shmem[indx].client = indx;
                            shmem[indx].pid = proc_pid;
                            char* prova = strcpy(shmem[indx].name, (dir_list->list[indx]));

                            //semOp(semid_sync, SYNC_SHM, SIGNAL, 0);

                            printf("\n\nho scritto su shared memory: %s\n\n", prova);

                            // marco la zona di memoria come piena
                            supporto[indx] = 1; // sull'array di supporto
                            matrix_msg[child][SHM] = 1; // e sulla matrice

                            if (parts[SHM] != NULL)
                                strcpy(shmem[indx].message, parts[SHM]);
                            else{
                                strcpy(shmem[indx].message, "sbagliato");
                                printf("\t→ <Client-%zu>: parts[%d] è NULL\n", child, SHM);
                            }

                            printf("\t→ <Client-%zu>: Ho inviato il quarto pezzo del file <%s> su Shared Memory\n",
                                   child, dir_list->list[child]);
                        }
                        else {
                            printf("\t→ <Client-%zu>: Shared memory piena! [%d]: %s\n", child, errno, strerror(errno));
                        }

                        // sblocca server per la lettura
                        semOp(semid_sync, SEMSHM, SIGNAL, 0);
                        printf("\t→ <Client-%zu>: Ho liberato il mutex Shared Memory!\n", child);
                    }

                    //sleep(1);
                }
                child2_finish(matrix_msg , child);
            }

            // chiude il file
            close(sendme_fd);

            // termina
            printf("\n\t→ <Client-%zu>: Ho finito di inviare il file <%s>\n\n", child + 1, dir_list->list[child]);

            exit(EXIT_SUCCESS);
        }
        else {
            if (child == 0){
                printf("→ <Client-0>: Waiting all child...\n");
            }
        }
    }

    // Parent wait children
    while(wait(NULL) > 0);
    // Client-0 wait for server ack
    msg_t result;
    printf("→ <Client-0>: waiting for server ack...\n");
    //semOp(semid_sync, SEMMSQ, SIGNAL, 0);
    // TODO secondo me qui ci va un semaforo per la sincroizzazione
    semOp(semid_sync, SEMMSQ, WAIT, 0);

    msgrcv(msqid, &result, 0, MSGSIZE, 0);

    semOp(semid_sync, SEMMSQ, SIGNAL, 0);

    printf("%s", result.message);

    //then close all the IPCs
    close_fd(fifo1_fd);
    close_fd(fifo2_fd);
    free_shared_memory(shmem);
    free_shared_memory(supporto);


    // --------------Finish--------------
    // free heap
    for (size_t i = 0; i < dir_list->index; i++) {
        free(dir_list->list[i]);
    }
    free(dir_list);
    /* ################################################################################### */

    // set old mask
    printf("→ <Client-0>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
    sig_setmask(SIG_SETMASK, &oldSet, NULL);
    // wait for a signal
    pause();
}
