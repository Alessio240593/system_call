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
    printf("\n\t→ <Client-0>: Received signal %s\n\n", signame[sig]);
    exit(EXIT_SUCCESS);
}

void sigint_handler(int sig)
{
    //notify signal
    printf("\n\t→ <Client-0>: Received signal %s\n\n", signame[sig]);
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

    char buffer[LEN_INT];
    
    // get message queue
    printf("→ <Client-0>: Waiting message queue n°%d synchronization...\n", KEYMSQ);
    int msqid = get_message_queue(KEYMSQ);

    //get server shmem
    printf("→ <Client-0>: Waiting shared memory n°%d synchronization...\n", KEYSHM);
    int shmid = get_shared_memory(KEYSHM, SHMSIZE);
    msg_t *shmem = (msg_t *)attach_shared_memory(shmid, 0);

    //get server semset
    printf("→ <Client-0>: Waiting semaphore set n°%d synchronization...\n", KEYSEM_SYNC);
    int semid_sync = get_semaphore(KEYSEM_SYNC , SEMNUM_SYNC);

    //get sem count
    printf("→ <Client-0>: Waiting semaphore set n°%d synchronization...\n", KEYSEM_COUNTER);
    int semid_counter = get_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);


    pid_t proc_pid = getpid();
    msg_t pid_msg;
    pid_msg.type = 1;
    pid_msg.pid = proc_pid;

    if (msgsnd(msqid, &pid_msg, MSGSIZE, 0) == -1) {
        errExit("failed to send pid to server: ");
    }

    while (1) {
        //wait for a signal
        pause();

        // fill sigset
        sig_fillset(&mySet);

        // set signal mask
        printf("→ <Client-0>: Setting new mask UNBLOCK<NONE>...\n\n");
        sig_setmask(SIG_SETMASK, &mySet, &oldSet);


        //change working directory
        Chdir(path);

        printf("→ <Client-0>: Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), getenv("PWD"));

        /* dirlist declaration and initialization */
        dirlist_t *dir_list = (dirlist_t *) malloc(sizeof(dirlist_t));
        MCHECK(dir_list);

        dir_list->index = 0;
        dir_list->size = 1;
        dir_list->list = (char **) calloc(dir_list->size, sizeof(char *));
        MCHECK(dir_list->list);

        init_dirlist(dir_list, path);

#ifndef _DEBUG
        dump_dirlist(dir_list, "before.txt");

#endif

        //open FIFO_1 in write only mode
        int fifo1_fd = open(FIFO_1, O_WRONLY | O_NONBLOCK);
        SYSCHECK(fifo1_fd, "open1: ");

        snprintf(buffer, LEN_INT, "%zu", dir_list->size);

        // write on fifo
        write_fifo(fifo1_fd, buffer, LEN_INT);

        semOp(semid_sync, SYNC_FIFO1, SIGNAL);

        close(fifo1_fd);

        // waiting data
        semOp(semid_sync, SYNC_SHM, WAIT);

        // print shmem data
        printf("%s", shmem->message);

        // start creation
        size_t child;
        pid_t pid;

        // matrice del server per la memorizzazione dei messaggi
        //int matrix_msg = (int **) calloc(dir_list->size, sizeof(int *));

        int matrix_msg[37][4] = {0};

        //allocazione di 4 colonne per ogni client che conterrà la parte 1...4
        /*for (size_t i = 0; i < dir_list->size; i++) {
            matrix_msg[i] = (int *) calloc(PARTS, sizeof(int));
            MCHECK(matrix_msg[i]);
        }*/

        char **parts = (char **) calloc(PARTS, sizeof(char *));
        MCHECK(parts);

        for (child = 0; child < dir_list->index; child++) {
            pid = fork();

            if (pid < 0) {
                fprintf(stderr, "\t→ <Client-%zu> Not created", child + 1);
                errExit("fork failed: ");
            } else if (pid == 0) {
                pid_t proc_pid;
                ssize_t tot_char;
                int waiting;
                // to store result
                int res = 0;
                // to store byte read
                ssize_t Br = 0;
                struct sembuf sop[PARTS]; // counter to MAX
                //struct sembuf msq_sync;
                struct sembuf shm_sync;
                msg_t msgs[PARTS];

                //printf("\n\nshmid: %d, msgid: %d\n\n", shmid, msqid);

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
                    semOp(semid_sync, SEMCHILD, WAIT);
                } else {
                    // Figli diseredati
                    semOp(semid_sync, SEMCHILD, SYNC);
                }

                // start sending messages
                proc_pid = getpid();

                while (child_finish(matrix_msg, child)) {
                    //-----------------------FIFO_1-----------------------------
                    if (matrix_msg[child][FIFO1] != 1) {
                        int fifo1_fd = open(FIFO_1, O_WRONLY | O_NONBLOCK);
                        SYSCHECK(fifo1_fd, "open1: ");

                        // semaforo per tenere traccia delle scritture sulla FIFO_1
                        sop[FIFO1].sem_num = MAX_SEM_FIFO1;
                        sop[FIFO1].sem_op = WAIT;
                        sop[FIFO1].sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_counter, &sop[FIFO1], 1);

                        if (res == -1 && errno == EAGAIN) {
                            //printf("\t→ <Client-%zu>: FIFO_1 non disponibile, %s\n", child + 1, strerror(errno));
                            semOp(semid_counter, MAX_SEM_FIFO1, IPC_NOWAIT);
                        } else if (res == -1) {
                            errExit("semop failed: ");
                        } else {
                            //preparo il messaggio da mandare su fifo1
                            msg_t FIFO1_msg;
                            FIFO1_msg.type = 1;
                            FIFO1_msg.client = child;
                            FIFO1_msg.pid = proc_pid;
                            strcpy(FIFO1_msg.name, (dir_list->list[child]));
                            strcpy(FIFO1_msg.message, parts[FIFO1]);
                            //printf("fifo 1 type: %ld\n", FIFO1_msg.type);
                            //printf("fifo 1 name: %s\n", FIFO1_msg.name);
                            //printf("fifo 1 client: %ld\n", FIFO1_msg.client );
                            //printf("fifo 1 messange: %s\n", FIFO1_msg.message);

                            errno = 0;

                            Br = write(fifo1_fd, &FIFO1_msg, sizeof(FIFO1_msg));

                            if (Br == -1 && errno == EAGAIN) {
                                //printf("\t→ <Client-%zu>: FIFO_1 piena!\n", child + 1);
                                semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                            } else if (Br == -1) {
                                errExit("write failed: ");
                            } else {
                                if (Br > 0) {
                                    matrix_msg[child][FIFO1] = 1;
                                    //printf("\t→ <Client-%zu>: Ho inviato il primo pezzo del file <%s> sulla FIFO_1\n",
                                    //child + 1, dir_list->list[child]);
                                    //printf("Il processo %ld manda in fifo1: <%s> \n", FIFO1_msg.client,
                                           //FIFO1_msg.message);
                                } else {
                                    semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                                    //printf("<Client%zu>HO LETTO 0\n", child);
                                }
                            }
                        }
                        close(fifo1_fd);
                    }

                    //-----------------------FIFO_2-----------------------------
                    if (matrix_msg[child][FIFO2] != 1) {
                        //open FIFO_2 in write only mode
                        int fifo2_fd = open(FIFO_2, O_WRONLY | O_NONBLOCK);
                        SYSCHECK(fifo2_fd, "open2: ");

                        // semaforo per tenere traccia delle scritture sulla FIFO_2
                        sop[FIFO2].sem_num = MAX_SEM_FIFO2;
                        sop[FIFO2].sem_op = WAIT;
                        sop[FIFO2].sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_counter, &sop[FIFO2], 1);

                        if (res == -1 && errno == EAGAIN) {
                            //printf("\t→ <Client-%zu>: FIFO_2 non disponibile, %s\n", child + 1, strerror(errno));
                            semOp(semid_counter, MAX_SEM_FIFO2, IPC_NOWAIT);
                        } else if (res == -1) {
                            errExit("semop failed: ");
                        } else {
                            //preparo il messaggio da mandare su FIFO2
                            msg_t FIFO2_msg;
                            FIFO2_msg.type = 1;
                            FIFO2_msg.client = child;
                            FIFO2_msg.pid = proc_pid;
                            strcpy(FIFO2_msg.name, (dir_list->list[child]));
                            strcpy(FIFO2_msg.message, parts[FIFO2]);

                            //printf("fifo 2 type: %ld\n", FIFO2_msg.type);
                            //printf("fifo 2 name: %s\n", FIFO2_msg.name);
                            //printf("fifo 2 client: %ld\n", FIFO2_msg.client );
                            //printf("fifo 2 messange: %s\n", FIFO2_msg.message);

                            errno = 0;

                            //printf("\t→ <Client-%zu>: sono bloccato in scrittura su FIFO_2!\n", child + 1);
                            Br = write(fifo2_fd, &FIFO2_msg, sizeof(FIFO2_msg));

                            if (Br == -1 && errno == EAGAIN) {
                                //printf("\t→ <Client-%zu>: FIFO_2 piena!\n", child + 1);
                                semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                            } else if (Br == -1) {
                                errExit("write failed: ");
                            } else {
                                if (Br > 0) {
                                    matrix_msg[child][FIFO2] = 1;
                                    //printf("\t→ <Client-%zu>: Ho inviato il secondo pezzo del file <%s> sulla FIFO_2\n",
                                    //child + 1, dir_list->list[child]);
                                    //printf("Il processo %ld manda in fifo2: <%s> \n", FIFO2_msg.client,
                                           //FIFO2_msg.message);
                                } else {
                                    semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                                    //printf("<Client%zu>HO LETTO 0\n", child);
                                }
                            }
                        }
                        close(fifo2_fd);
                    }


                    //-----------------------MESSAGE QUEUE-----------------------------
                    if (matrix_msg[child][MSQ] != 1) {
                        // semaforo per tenere traccia delle scritture sulla MSQ
                        sop[MSQ].sem_num = MAX_SEM_MSQ;
                        sop[MSQ].sem_op = WAIT;
                        sop[MSQ].sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_counter, &sop[MSQ], 1);

                        if (res == -1 && errno == EAGAIN) {
                            //printf("\t→ <Client-%zu>: Message Queue non disponibile [%d], %s\n",
                            //child + 1, errno, strerror(errno));
                            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
                        } else if (res == -1) {
                            //printf("sono morto e non lo sapevo");
                            errExit("semop failed: ");
                        } else {
                            // preparo il messaggio da mandare su MSQ
                            msgs[MSQ].type = 1;
                            msgs[MSQ].client = child;
                            msgs[MSQ].pid = proc_pid;
                            strcpy(msgs[MSQ].name, (dir_list->list[child]));
                            strcpy(msgs[MSQ].message, parts[MSQ]);

                            errno = 0;

                            res = msgsnd(msqid, &msgs[MSQ], MSGSIZE, IPC_NOWAIT);

                            if (res == -1 && errno == EAGAIN) {
                                //printf("\t→ <Client-%zu>: Message queue piena!\n", child + 1);
                                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
                            } else if (res == -1) {
                                errExit("msgsnd failed: ");
                            } else {
                                matrix_msg[child][MSQ] = 1;

                                //printf("\t→ <Client-%zu>: Sono uscito dal mutex di msq!\n", child + 1);

                                //printf("\t→ <Client-%zu>: Ho inviato il terzo pezzo del file <%s> su Message Queue\n",
                                //child + 1, dir_list->list[child]);
                            }
                        }
                    }
                    //}


                    //-----------------------SHARED MEMORY-----------------------------
                    if (matrix_msg[child][SHM] != 1) {
                        // inizio sezione critica
                        shm_sync.sem_num = SEMSHM;
                        shm_sync.sem_op = WAIT;
                        shm_sync.sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_sync, &shm_sync, 1);
                        // TODO controllare errore -1
                        if (res == 0) {
                            //printf("child n°: %ld\n", child);
                            for (int id = 0; id < MAXMSG; id++) {
                                if (shmem[id].type == 0) {
                                    // preparo il messaggio da inviare sulla Shared Memory
                                    msg_t *shm_msg = (msg_t *) malloc(sizeof(msg_t));
                                    MCHECK(shm_msg);
                                    //printf("Sono il pocesso:  %d\n", proc_pid);
                                    // scrivo sula shared memory
                                    shmem[id].type = 1;
                                    shmem[id].client = child;
                                    shmem[id].pid = proc_pid;
                                    strcpy(shmem[id].name, (dir_list->list[child]));

                                    matrix_msg[child][SHM] = 1; // e sulla matrice

                                    if (parts[SHM] != NULL)
                                        strcpy(shmem[id].message, parts[SHM]);
                                    else {
                                        strcpy(shmem[id].message, "sbagliato");
                                        //printf("\t→ <Client-%zu>: parts[%d] è NULL\n", child, SHM);
                                    }

                                    //printf("\t→ <Client-%zu>: Ho inviato il quarto pezzo del file <%s> su Shared Memory\n",
                                    //child, dir_list->list[child]);
                                    id = MAXMSG;
                                }
                                //else {
                                //printf("\t→ <Client-%zu>: Shared memory piena! [%d]: %s\n", child, errno, strerror(errno));
                                //}

                                // sblocca server per la lettura

                                //printf("\t→ <Client-%zu>: Ho liberato il mutex Shared Memory!\n", child);

                            }
                            semOp(semid_sync, SEMSHM, SIGNAL);
                        }
                    }
                }

                // chiude il file
                close(sendme_fd);
                // termina
                printf("\t→ <Client-%zu>: Ho finito di inviare il file <%s>\n", child + 1, dir_list->list[child]);

                exit(EXIT_SUCCESS);
            } else {
                if (child == 0) {
                    printf("→ <Client-0>: Waiting all child...\n");
                }
            }
        }

        // Parent wait children
        while (wait(NULL) > 0);
        // Client-0 wait for server ack
        msg_t result;
        printf("\n→ <Client-0>: waiting for server ack...\n\n");
        //semOp(semid_sync, SEMMSQ, SIGNAL, 0);
        semOp(semid_sync, SYNCH_MSQ, WAIT);

        msgrcv(msqid, &result, MSGSIZE, 0, 0);

        printf("%s", result.message);

        // --------------Finish--------------
        // free heap
        for (size_t i = 0; i < dir_list->index; i++) {
            free(dir_list->list[i]);
        }

        for (size_t i = 0; i < PARTS; i++) {
            free(parts[i]);
        }

        free(parts);
        free(dir_list->list);
        free(dir_list);

        // set old mask
        printf("→ <Client-0>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
        sig_setmask(SIG_SETMASK, &oldSet, NULL);
        printf("\n→ <Client-0>: Waiting <ctrl+c> SIGINT...\n");
    }
}
