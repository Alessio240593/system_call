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
#include <string.h>

extern const char *signame[];
const char *path;

void sigusr1_handler(int sig)
{
    printf("\n\t→ <Client-0>: Received signal %s\n\n", signame[sig]);
    printf("→ <Client-0>: Exit success!\n");
    exit(EXIT_SUCCESS);
}

void sigint_handler(int sig)
{
    //notify signal
    printf("\t→ <Client-0>: Received signal %s\n\n", signame[sig]);
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

    // set path
    path = strdup(argv[1]);

    // create an empty signal set
    sigset_t mySet;
    sigset_t oldSet;

    // fill sigset
    sig_fillset(&mySet);

    // delete SIGINT and SIGUSR1 from the set
    sig_remove(&mySet, 2, SIGINT, SIGUSR1);

    // set mask
    printf("→ <Client-0>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
    sig_setmask(SIG_SETMASK, &mySet, NULL);
    // set signal handler
    sig_sethandler(SIGINT, sigint_handler);
    sig_sethandler(SIGUSR1, sigusr1_handler);


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

    // big declaration
    char buffer[LEN_INT];
    dirlist_t *dir_list;
    int **matrix_msg;
    char parts[PARTS][1025];
    msg_t result;
    msg_t msgs[PARTS];
    struct sembuf sop[PARTS]; // counter to MAX
    struct sembuf shm_sync;
    int waiting;
    int res = 0;
    ssize_t Br = 0;
    ssize_t tot_char;
    int fifo2_fd = 0;
    int fifo1_fd = 0;
    int sendme_fd = 0;
    size_t child;
    pid_t pid;

    while (1) {
        //wait for a signal
        pause();

        // fill sigset
        sig_fillset(&mySet);

        // set signal mask
        printf("→ <Client-0>: Setting new mask UNBLOCK<NONE>...\n\n");
        sig_setmask(SIG_SETMASK, &mySet, &oldSet);

        // change working directory
        Chdir(path);

        printf("→ <Client-0>: Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), getenv("PWD"));

        // dirlist declaration and initialization
        dir_list = (dirlist_t *) malloc(sizeof(dirlist_t));
        MCHECK(dir_list);

        dir_list->index = 0;
        dir_list->size = 100;
        dir_list->list = (char **) calloc(dir_list->size, sizeof(char *));
        MCHECK(dir_list->list);
        init_dirlist(dir_list, path);
        // open FIFO_1 in write only mode
        fifo1_fd = open(FIFO_1, O_WRONLY | O_NONBLOCK);
        SYSCHECK(fifo1_fd, "open: ");

        snprintf(buffer, LEN_INT, "%zu", dir_list->index);

        // write on fifo
        write_fifo(fifo1_fd, buffer, LEN_INT);

        semOp(semid_sync, SYNC_FIFO1, SIGNAL);

        close(fifo1_fd);

        // waiting data
        semOp(semid_sync, SYNC_SHM, WAIT);

        // print shmem data
        printf("%s", shmem->message);

        // matrice del server per la memorizzazione dei messaggi
        matrix_msg = (int **) calloc(dir_list->index, sizeof(int *));
        MCHECK(matrix_msg);

        for (size_t i = 0; i < dir_list->index; i++) {
            matrix_msg[i] = (int *) malloc(sizeof(int));
            MCHECK(matrix_msg[i]);
        }

        // start creation
        for (child = 0; child < dir_list->index; child++) {
            pid = fork();

            if (pid < 0) {
                fprintf(stderr, "\t→ <Client-%zu> Not created", child + 1);
                errExit("fork failed: ");
            } else if (pid == 0) {
                // child-th child
                sendme_fd = open(dir_list->list[child], O_RDONLY);
                SYSCHECK(sendme_fd, "opensendme");

                tot_char = count_char(sendme_fd);
                split_file(parts, sendme_fd, tot_char);

                waiting = semctl(semid_sync, SEMCHILD, GETZCNT, 0);
                if (waiting == -1) {
                    errExit("semctl failed: ");
                } else if (waiting == 0 && dir_list->index == 1) {
                    // no-OP
                } else if (waiting == (int) (dir_list->index - 1)) {
                    // Figlio prediletto forse è -1 perchè conta child file in attesa
                    semOp(semid_sync, SEMCHILD, WAIT);
                } else {
                    // Figli diseredati
                    semOp(semid_sync, SEMCHILD, SYNC);
                }

                // start sending messages
                proc_pid = getpid();

                // invio messaggi sulle IPCs
                while (has_child_finished(matrix_msg, child))
                {
                    //-----------------------FIFO_1-----------------------------
                    if (matrix_msg[child][FIFO1] != 1) {
                        fifo1_fd = open(FIFO_1, O_WRONLY | O_NONBLOCK);
                        SYSCHECK(fifo1_fd, "open1: ");

                        // semaforo per tenere traccia delle scritture sulla FIFO_1
                        sop[FIFO1].sem_num = MAX_SEM_FIFO1;
                        sop[FIFO1].sem_op = WAIT;
                        sop[FIFO1].sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_counter, &sop[FIFO1], 1);

                        if (res == -1 && errno == EAGAIN) {
                            //no-Op
                        } else if (res == -1) {
                            errExit("semop failed: ");
                        } else {
                            //preparo il messaggio da mandare su fifo1
                            msgs[FIFO1].type = 1;
                            msgs[FIFO1].client = child;
                            msgs[FIFO1].pid = proc_pid;
                            strcpy(msgs[FIFO1].name, (dir_list->list[child]));

                            if (parts[FIFO1] != NULL) {
                                strcpy(msgs[FIFO1].message, parts[FIFO1]);
                            } else {
                                strcpy(msgs[FIFO1].message, "");
                            }

                            errno = 0;

                            Br = write(fifo1_fd, &msgs[FIFO1], sizeof(msgs[FIFO1]));

                            if (Br == -1 && errno == EAGAIN) {
                                semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                            } else if (Br == -1) {
                                errExit("write failed: ");
                            } else {
                                if (Br > 0) {
                                    matrix_msg[child][FIFO1] = 1;
                                } else {
                                    semOp(semid_counter, MAX_SEM_FIFO1, SIGNAL);
                                }
                            }
                        }
                        close(fifo1_fd);
                    }

                    //-----------------------FIFO_2-----------------------------
                    if (matrix_msg[child][FIFO2] != 1) {
                        fifo2_fd = open(FIFO_2, O_WRONLY | O_NONBLOCK);
                        SYSCHECK(fifo2_fd, "open2: ");

                        // semaforo per tenere traccia delle scritture sulla FIFO_2
                        sop[FIFO2].sem_num = MAX_SEM_FIFO2;
                        sop[FIFO2].sem_op = WAIT;
                        sop[FIFO2].sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_counter, &sop[FIFO2], 1);

                        if (res == -1 && errno == EAGAIN) {
                            ////no-Op
                        } else if (res == -1) {
                            errExit("semop failed: ");
                        } else {
                            //preparo il messaggio da mandare su FIFO2
                            msgs[FIFO2].type = 1;
                            msgs[FIFO2].client = child;
                            msgs[FIFO2].pid = proc_pid;
                            strcpy(msgs[FIFO2].name, (dir_list->list[child]));

                            if (parts[FIFO2] != NULL)
                                strcpy(msgs[FIFO2].message, parts[FIFO2]);
                            else {
                                strcpy(msgs[FIFO2].message, "");
                            }

                            errno = 0;

                            Br = write(fifo2_fd, &msgs[FIFO2], sizeof(msgs[FIFO2]));

                            if (Br == -1 && errno == EAGAIN) {
                                semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
                            } else if (Br == -1) {
                                errExit("write failed: ");
                            } else {
                                if (Br > 0) {
                                    matrix_msg[child][FIFO2] = 1;
                                } else {
                                    semOp(semid_counter, MAX_SEM_FIFO2, SIGNAL);
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
                            semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
                        } else if (res == -1) {
                            errExit("semop failed: ");
                        } else {
                            // preparo il messaggio da mandare su MSQ
                            msgs[MSQ].type = 1;
                            msgs[MSQ].client = child;
                            msgs[MSQ].pid = proc_pid;
                            strcpy(msgs[MSQ].name, (dir_list->list[child]));

                            if (parts[MSQ] != NULL)
                                strcpy(msgs[MSQ].message, parts[MSQ]);
                            else {
                                strcpy(msgs[MSQ].message, "");
                            }

                            errno = 0;

                            res = msgsnd(msqid, &msgs[MSQ], MSGSIZE, IPC_NOWAIT);

                            if (res == -1 && errno == EAGAIN) {
                                semOp(semid_counter, MAX_SEM_MSQ, SIGNAL);
                            } else if (res == -1) {
                                errExit("msgsnd failed: ");
                            } else {
                                matrix_msg[child][MSQ] = 1;
                            }
                        }
                    }


                    //-----------------------SHARED MEMORY-----------------------------
                    if (matrix_msg[child][SHM] != 1) {
                        // inizio sezione critica
                        shm_sync.sem_num = SEMSHM;
                        shm_sync.sem_op = WAIT;
                        shm_sync.sem_flg = IPC_NOWAIT;

                        errno = 0;

                        res = semop(semid_sync, &shm_sync, 1);

                        if (res == 0) {
                            for (int id = 0; id < MAXMSG; id++) {
                                if (shmem[id].type == 0) {
                                    // scrivo sula shared memory
                                    shmem[id].type = 1;
                                    shmem[id].client = child;
                                    shmem[id].pid = proc_pid;
                                    strcpy(shmem[id].name, (dir_list->list[child]));

                                    matrix_msg[child][SHM] = 1; // e sulla matrice

                                    if (parts[SHM] != NULL)
                                        strcpy(shmem[id].message, parts[SHM]);
                                    else {
                                        strcpy(shmem[id].message, "");
                                    }

                                    id = MAXMSG;
                                }
                            }
                            // sblocca server per la lettura
                            semOp(semid_sync, SEMSHM, SIGNAL);
                        } else if (res == -1 && errno != EAGAIN) {
                            errExit("\tSHMEM semop failed : ");
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
        printf("\n→ <Client-0>: waiting for server ack...\n\n");
        semOp(semid_sync, SYNCH_MSQ, WAIT);

        msgrcv(msqid, &result, MSGSIZE, 0, 0);

        printf("%s", result.message);

        // ------------------FINISH------------------
        // free HEAP
        for (size_t i = 0; i < dir_list->index; i++) {
            free(dir_list->list[i]);
            free(matrix_msg[i]);
        }

        /*for (size_t i = 0; i < PARTS; i++) {
            if (parts[i] != NULL)
                free(parts[i]);
        }*/

        //free(parts);
        free(matrix_msg);
        free(dir_list->list);
        free(dir_list);


        // set old mask
        printf("→ <Client-0>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
        sig_setmask(SIG_SETMASK, &oldSet, NULL);
        printf("\n→ <Client-0>: Waiting <ctrl+c> SIGINT...\n");
    }
}
