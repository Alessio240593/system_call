/** @file client.c
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
    printf("→ <Client>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
    sig_setmask(SIG_SETMASK, &mySet, NULL);
    //set signal handler
    sig_sethandler(SIGINT, sigint_handler);
    sig_sethandler(SIGUSR1, sigusr1_handler);

    //wait for a signal
    //pause();

    char buffer[LEN_INT];

    //open FIFO1 in write only mode
    int fifo1_fd = open_fifo(FIFO1, O_WRONLY, 0);
    SYSCHECK(fifo1_fd, "open: ");

    //get server shmem
    printf("→ <Client>: Waiting shared memory synchronization...\n");
    int shmid = get_shared_memory(KEYSHM, SHMSIZE);
    msg_t *shmem = (msg_t *)attach_shared_memory(shmid, 0);

    //get supporto
    printf("→ <Client>: Waiting support shared memory synchronization...\n");
    int support_id = get_shared_memory(KEY_SUPPORT, SUPPORTO_SIZE);
    int *supporto = (int *)attach_shared_memory(support_id, 0);

    // get message queue
    printf("→ <Client>: Waiting message queue synchronization...\n");
    int msqid = get_message_queue(KEYMSQ);

    //get server semset
    printf("→ <Client>: Waiting semaphore set synchronization...\n");
    int semid_sync = get_semaphore(KEYSEM_SYNC , SEMNUM_SYNC);

    //get sem count
    printf("→ <Client>: Waiting semaphore counter set synchronization...\n");
    int semid_counter = get_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    // fill sigset
    sig_fillset(&mySet);

    // set signal mask
    printf("→ <Client>: Setting new mask UNBLOCK<NONE>...\n");
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

#ifndef DEBUG
    dump_dirlist(dir_list, "before.txt");

    /* printf("index (%zu) == size (%zu) ? %s\n", dir_list->index, dir_list->size,
           dir_list->index == dir_list->size ? "sì" : "no");
    */
#endif

    snprintf(buffer, LEN_INT, "%zu", dir_list->size);

    // open FIFO2 in write only mode
    int fifo2_fd = open(FIFO2, O_WRONLY | O_NONBLOCK);
    SYSCHECK(fifo2_fd, "open: ");

    // write on fifo
    write_fifo(fifo1_fd, buffer, LEN_INT);

    close(fifo1_fd);

    //open FIFO1 in write only mode
    fifo1_fd = open(FIFO1, O_WRONLY | O_NONBLOCK);
    SYSCHECK(fifo1_fd, "open: ");

    // waiting data
    semOp(semid_sync, SYNC_SHM, WAIT, 0);

    // print shmem data
    printf("%s", shmem->message);

    // start creation
    size_t i;
    pid_t pid;
    pid_t proc_pid;
    ssize_t tot_char;
    int waiting;

    //matrice del server per la memorizzazione dei messaggi
    int **msg_map = (int **) calloc(dir_list->size, sizeof(int *));

    //allocazione di 4 colonne per ogni client che conterrà la parte 1...4
    for (size_t i = 0; i < dir_list->size; i++) {
        msg_map[i] = (int *) calloc(PARTS, sizeof(int));
        MCHECK(msg_map[i]);
    }

    char **parts = (char **) calloc(PARTS, sizeof(char *));
    MCHECK(parts);

    for (i = 0; i < dir_list->index; i++) {
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "\t→ <Client-%zu> Not created", i);
            errExit("fork failed: ");
        }
        else if (pid == 0) {
            int sendme_fd = open(dir_list->list[i], O_RDONLY);
            SYSCHECK(sendme_fd, "open");

            // i-th child
            tot_char = count_char(sendme_fd);
            split_file(parts, sendme_fd, tot_char);
            split_file(parts, sendme_fd, tot_char);

            waiting = semctl(semid_sync, SEMCHILD, GETZCNT, 0);
            if (waiting == -1) {
                errExit("semctl failed: ");
            } else if (waiting == (int) (dir_list->index - 1)) {
                // Figlio prediletto forse è -1 perchè conta i file in attesa
                //semOp(semid_sync, SEMCHILD, WAIT, 0);
            } else {
                // Figli diseredati
                //semOp(semid_sync, SEMCHILD, SYNC, 0);
            }

            // start sending messages
            proc_pid = getpid();

            while(child_finish(msg_map , i)) {
                //-----------------------FIFO1-----------------------------


                // semaforo per tenere traccia delle scritture sulla FIFO1
                struct sembuf sop = {.sem_num = MAX_SEM_FIFO1, .sem_op = WAIT, .sem_flg = IPC_NOWAIT};

                errno = 0;

                int res = semop(semid_counter, &sop, 1);

                if(res == -1 && errno == EAGAIN)
                {
                    printf("→ <Client%zu>: Fifo1 non disponibile, %s\n", i,
                           strerror(errno));
                }
                else if(res == -1){
                    errExit("semop failed: ");
                } else {
                    //preparo il messaggio da mandare su fifo1
                    msg_t fifo1_msg;
                    fifo1_msg.type = 1;
                    fifo1_msg.client = i;
                    fifo1_msg.pid = proc_pid;
                    strcpy(fifo1_msg.name, (dir_list->list[i]));
                    strcpy(fifo1_msg.message, parts[0]);

                    printf("sono bloccato in scrittura su fifo1!\n");
                    ssize_t Br = write_fifo(fifo1_fd, &fifo1_msg, sizeof(fifo1_msg));

                    if (Br == -1 && errno == EAGAIN) {
                        printf("→ <Server>: fifo1 piena!\n");
                    }
                    else if(Br == -1) {
                        errExit("read failed: ");
                    }
                    else{
                        msg_map[i][0] = 1;
                        printf("\t→ <Client-%zu>: Ho inviato il primo pezzo del file <%s> sulla FIFO1\n", i + 1,
                               dir_list->list[i]);
                    }
                }

                //-----------------------FIFO2-----------------------------
                // preparo il messaggio da mandare su fifo2
                msg_t fifo2_msg;
                fifo2_msg.type = 1;
                fifo2_msg.client = i;
                fifo2_msg.pid = proc_pid;
                strcpy(fifo2_msg.name, (dir_list->list[i]));
                strcpy(fifo2_msg.message, parts[1]);

                // semaforo per tenere traccia delle scritture sulla FIFO2
                semOp(semid_counter, MAX_SEM_FIFO2, WAIT, 0); // TODO bloccano tutto
                printf("sono bloccato in scrittura su fifo2!\n");
                write_fifo(fifo2_fd, &fifo2_msg, sizeof(fifo2_msg));

                printf("\t→ <Client-%zu>: Ho inviato il secondo pezzo del file <%s> sulla FIFO2\n", i + 1,
                       dir_list->list[i]);

                //-----------------------MESSAGE QUEUE-----------------------------
                // TODO problemi nella msqueue
                msg_t msq_msg;
                msq_msg.type = 1;
                msq_msg.client = i;
                msq_msg.pid = proc_pid;
                strcpy(msq_msg.name, (dir_list->list[i]));
                strcpy(msq_msg.message, parts[2]);

                // semaforo per tenere traccia delle scritture sulla Message Queue
                sop = {.sem_num = MAX_SEM_MSQ, .sem_op = WAIT, .sem_flg = IPC_NOWAIT};

                errno = 0;

                res = semop(semid_counter, &sop, 1);

                if(res == -1 && errno == EAGAIN)
                {
                    printf("→ <Client%zu>: Message queue non disponibile, %s\n", i,
                           strerror(errno));
                }
                else if(res == -1){
                    errExit("semop failed: ");
                } else {

                    // inizio sezione critica
                    printf("Sto per prendere il mutex msq!\n");

                    struct sembuf sop1 = {.sem_num = SEMMSQ, .sem_op = WAIT, .sem_flg = IPC_NOWAIT};

                    errno = 0;

                    res = semop(semid_sync, &sop1, 1);

                    if(res == -1 && errno == EAGAIN)
                    {
                        printf("→ <Client%zu>: Message queue mutex occupato");
                    }
                    else if(res == -1){
                        errExit("semop failed: ");
                    } else {

                        res = msgsnd(msqid, &msq_msg, MSGSIZE, IPC_NOWAIT);

                        if (res == -1 && errno == EINVAL) {
                            printf("→ <Client%zu>: Message queue piena!");
                        }
                        else if(res == -1){
                            errExit("msgsnd failed: ");
                        } else {
                            msg_map[i][2] = 1;
                            semOp(semid_sync, SEMMSQ, SIGNAL, 0);
                            printf("Sono uscito dal mutex di msq!\n");

                            printf("\t→ <Client-%zu>: Ho inviato il terzo pezzo del file <%s> su Message Queue\n",
                                   i + 1,
                                   dir_list->list[i]);
                        }
                    }
                }


                //-----------------------SHARED MEMORY-----------------------------
                msg_t *shm_msg = (msg_t *) malloc(sizeof(msg_t));
                MCHECK(shm_msg);

                // semaforo per tenere traccia delle scritture sulla Shared Memory
                //semOp(semid_counter, MAX_SEM_SHM, WAIT, 0); // TODO secondo me non serve se abbiamo supporto

                size_t index = 0;

                while (index < MAXMSG && supporto[index] == 1) {
                    index++;
                }
                // se non c'è spazio il client i aspetta
                if (index == MAXMSG) {

                    //res = semOp(semid_sync, SYNC_SHM, WAIT, 0); // TODO ipc no wait

                    //if(res == -1 && )
                }

                // inizio sezione critica
                printf("sto per prendere il mutex shmem!\n");
                semOp(semid_sync, SEMSHM, WAIT, 0);

                //marco la zona di memoria come piena
                supporto[index] = 1;

                //scrivo sula shared memory
                shmem[index].type = 1;
                shmem[index].client = i;
                shmem[index].pid = proc_pid;
                strcpy(shmem[index].name, (dir_list->list[i]));

                if (parts[3] != NULL)
                    strcpy(shmem[index].message, parts[3]);
                else
                    printf("parts[3] è NULL\n");

                printf("\t→ <Client-%zu>: Ho inviato il quarto pezzo del file <%s> su Shared Memory\n", i + 1,
                       dir_list->list[i]);

                semOp(semid_sync, SEMSHM, SIGNAL, 0);
                printf("Ho liberato il mutex shmem!\n");

                // sblocca server per la lettura
                //semOp(semid_sync, SYNC_SHM, SIGNAL, 0);
            }
            // chiude il file
            close(sendme_fd);

            // termina
            printf("\t→ <Client-%zu>: Ho finito di inviare il file <%s>\n", i+1, dir_list->list[i]);

            exit(EXIT_SUCCESS);
        }
        else{
            if(i == 0){
                printf("→ <Client-0>: Waiting all child...\n");
            }
        }
    }

    // Parent wait children
    while(wait(NULL) > 0);
    // Client-0 wait for server ack
    msg_t res;
    printf("<Client-0>: waiting for server ack...\n");
    //semOp(semid_sync, SEMMSQ, SIGNAL, 0);
    // TODO secondo me qui ci va un semaforo per la sincroizzazione
    semOp(semid_sync, SEMMSQ, WAIT, 0);

    msgrcv(msqid, &res, 0, MSGSIZE, 0);

    semOp(semid_sync, SEMMSQ, SIGNAL, 0);

    printf("%s", res.message);

    //then close all the IPCs
    close_fd(fifo1_fd);
    close_fd(fifo2_fd);
    free_shared_memory(shmem);
    free_shared_memory(supporto);

    /* -------------------- */
    //free heap
    for (size_t indx = 0; indx < dir_list->index; indx++) {
        free(dir_list->list[indx]);
    }
    free(dir_list);

    // set old mask
    printf("→ <Client>: Setting new mask UNBLOCK<SIGINT, SIGUSR1>...\n");
    sig_setmask(SIG_SETMASK, &oldSet, NULL);
    // wait for a signal
    pause();
}
