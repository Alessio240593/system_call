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

    sig_setmask(SIG_SETMASK, &mySet, NULL);

    /*for (int i = 0; i < 31; ++i) {
        printf("Il segnale %s : %s\n",signame[i], sigismember(&mySet, i) ? "si" : "no");
    }*/

    sig_sethandler(SIGINT, sigint_handler);
    sig_sethandler(SIGUSR1, sigusr1_handler);

    //wait for a signal
    pause();

    char buffer[LEN_INT];

    //open FIFO1 in write only mode
    errno = 0;
    int fifo1_fd = open(FIFO1, O_WRONLY);
    SYSCHECK(fifo1_fd, "open: ");

    //get server shmem
    int shmid = get_shared_memory(KEYSHM, SHMSIZE);
    msg_t **shmem = (msg_t **)attach_shared_memory(shmid, 0);

    // get message queue
    int msqid = get_message_queue(KEYMSQ);

    //get server semset
    int semid_sync = get_semaphore(KEYSEM_SYNC , SEMNUM_SYNC);
    //get sem count
    int semid_counter = get_semaphore(KEYSEM_COUNTER, SEMNUM_COUNTER);

    // fill sigset
    sig_fillset(&mySet);

    // set signal mask
    sig_setmask(SIG_SETMASK, &mySet, &oldSet);

    /*for (int i = 0; i < 31; ++i) {
        printf("Il segnale %s : %s\n",signame[i], sigismember(&mySet, i) ? "si" : "no");
    }*/

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

    // write on fifo
    //ssize_t bW = write(fifo1_fd, buffer, LEN_INT);
    //WCHECK(bW, LEN_INT);
    write_fifo(fifo1_fd, buffer, LEN_INT);

    // waiting data
    semOp(semid_sync, 0, WAIT);

    // print shmem data
    printf("%s", shmem[0]->message);
    shmem[0]->message = NULL;

    // start creation
    size_t i;
    pid_t pid;
    pid_t proc_pid;
    ssize_t tot_char;
    int waiting;
    size_t Br;

    char **parts = (char **) calloc(PARTS, sizeof(char *));
    MCHECK(parts);

//    struct mymsg *msq_msg = (struct mymsg *) malloc(sizeof(struct mymsg));
//    MCHECK(msq_msg);

    msg_t *shm_mem_locations = (msg_t *) calloc(MAXMSG, sizeof(msg_t));
    MCHECK(shm_mem_locations);

    // open FIFO2 in write only mode
    int fifo2_fd = open(FIFO2, O_WRONLY);
    SYSCHECK(fifo2_fd, "open: ");

    for (i = 0; i < dir_list->index; i++) {
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "<Client-%zu> Not created", i);
            errExit("fork failed: ");
        }
        else if (pid == 0) {
            // i-th child
            int sendme_fd = open(dir_list->list[i], O_RDONLY);
            SYSCHECK(sendme_fd, "open");

            tot_char = count_char(sendme_fd);
            split_file(parts, sendme_fd, tot_char);

            waiting = semctl(semid_sync, SEMCHILD, GETZCNT, 0);
            if (waiting == -1) {
                errExit("semctl failed: ");
            }
            else if (waiting == (int) (dir_list->index - 1)) {
                // Figlio prediletto forse è -1 perchè conta i file in attesa
                semOp(semid_sync, SEMCHILD, WAIT);
            }
            else {
                // Figli diseredati
                semOp(semid_sync, SEMCHILD, SYNC);
            }

            // start sending messages
            proc_pid = getpid();

            // old-TODO
            // polling del client tra le IPC:
            // se ne trova una bloccata non si ferma ma prova le altre

            msg_t fifo1_msg = {0, i,  proc_pid, dir_list->list[i], strdup(parts[0])};
            // semaforo per tenere traccia delle scritture sulla FIFO1
            semOp(semid_counter, MAX_SEM_FIFO1, WAIT);
            Br = write(fifo1_fd, &fifo1_msg, GET_MSG_SIZE(fifo1_msg));
            WCHECK(Br, GET_MSG_SIZE(fifo1_msg));
            printf("\t→ <Client-%zu>: Ho inviato il primo pezzo del file <%s> sulla FIFO1\n", i+1, dir_list->list[i]);

            msg_t fifo2_msg = {0, i,proc_pid, dir_list->list[i], strdup(parts[1])};
            // semaforo per tenere traccia delle scritture sulla FIFO2
            semOp(semid_counter, MAX_SEM_FIFO2, WAIT);
            Br = write(fifo2_fd, &fifo2_msg, GET_MSG_SIZE(fifo2_msg));
            WCHECK(Br, GET_MSG_SIZE(fifo2_msg));
            printf("\t→ <Client-%zu>: Ho inviato il secondo pezzo del file <%s> sulla FIFO2\n", i+1, dir_list->list[i]);

            msg_t msq_msg = {0, i, proc_pid, dir_list->list[i], strdup(parts[2])};
            // semaforo per tenere traccia delle scritture sulla Message Queue
            semOp(semid_counter, MAX_SEM_MSQ, WAIT);
            // inizio sezione critica
            semOp(semid_sync, SEMMSQ, WAIT);
            msg_send(msqid, &msq_msg, 0);
            semOp(semid_sync, SEMMSQ, SIGNAL);
            // fine sezione critica
            printf("\t→ <Client-%zu>: Ho inviato il terzo pezzo del file <%s> su Message Queue\n", i+1, dir_list->list[i]);


            msg_t shm_msg = {0, i, proc_pid, dir_list->list[i], strdup(parts[3])};
            // semaforo per tenere traccia delle scritture sulla Shared Memory
            // TODO
            semOp(semid_counter, MAX_SEM_SHM, WAIT);
            // inizio sezione critica
            semOp(semid_sync, SEMSHM, WAIT);
            // TODO completare la funzione
            //shmem_add(shmem, shm_msg);
            semOp(semid_sync, SEMSHM, SIGNAL);
            // fine sezione critica

            printf("\t→ <Client-%zu>: Ho inviato il quarto pezzo del file <%s> su Shared Memory\n", i+1, dir_list->list[i]);

            // chiude il file
            close(sendme_fd);

            // termina
            printf("\t→ <Client-%zu>: Ho finito di inviare il file <%s>\n", i+1, dir_list->list[i]);
            exit(EXIT_SUCCESS);
        }
        else{
            //parent code here!
            printf("Muovetevi evviva Gesù\n");
        }
    }
    close_fd(fifo1_fd);
    close_fd(fifo2_fd);

    //free_shared_memory(&shmem);
    while(wait(NULL) > 0);

    /* -------------------- */

    for (size_t indx = 0; indx < dir_list->index; indx++) {
        free(dir_list->list[indx]);
    }

    free(dir_list);
}
