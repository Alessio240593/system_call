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
    printf("\n\n → <Client-0>: Received signal %s\n", signame[sig]);
    exit(EXIT_SUCCESS);
}

void sigint_handler(int sig)
{
    //notify signal
    printf("\n\n → <Client-0>: Received signal %s\n", signame[sig]);
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
    else if (strlen(argv[1]) >= MAX_PATH) {
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

    //fill sigset
    sig_fillset(&mySet);

    //delete SIGINT and SIGUSR1 from the set
    sig_remove(&mySet, 2, SIGINT, SIGUSR1);

    sig_setmask(SIG_SETMASK, &mySet);

    /*for (int i = 0; i < 31; ++i) {
        printf("Il segnale %s : %s\n",signame[i], sigismember(&mySet, i) ? "si" : "no");
    }*/

    sig_sethandler(SIGINT, sigint_handler);
    sig_sethandler(SIGUSR1, sigusr1_handler);

    //wait for a signal
    pause();

    char buffer[LEN_INT];

    //open fifo in write only mode
    int fd1 = open(FIFO1, O_WRONLY);
    SYSCHECK(fd1, "open");

    //get server shmem
    int shmid = get_shared_memory(KEYSHM, SHMSIZE);
    char *shmem = (char *)attach_shared_memory(shmid, 0);

    // get message queue
    //int msqid = get_message_queue(MSGKEY);

    //get server semset
    int semid = get_semaphore(KEYSEM, SEMNUM);

    //fill sigset
    sig_fillset(&mySet);

    //set signal mask
    sig_setmask(SIG_SETMASK, &mySet);

    /*for (int i = 0; i < 31; ++i) {
        printf("Il segnale %s : %s\n",signame[i], sigismember(&mySet, i) ? "si" : "no");
    }*/

    //change working directory
    Chdir(path);

    printf("\n→ <Client-0>: Ciao %s, ora inizio l’invio dei file contenuti in %s\n\n", getenv("USER"), getenv("PWD"));

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

    /*printf("index (%zu) == size (%zu) ? %s\n", dir_list->index, dir_list->size,
           dir_list->index == dir_list->size ? "sì" : "no");*/
#endif

    snprintf(buffer, LEN_INT, "%zu", dir_list->size);

    //write on fifo
    ssize_t bW = write(fd1, buffer, LEN_INT);
    WCHECK(bW, LEN_INT);

    //waiting data
    semOp(semid,0,WAIT);

    //print shmem data
    printf("%s", shmem);

    // start creation
    size_t i;
    pid_t pid;
    ssize_t tot_char;
    int waiting;

    char **parts = (char **) calloc(PARTS, sizeof(char *));
    MCHECK(parts);

    struct mymsg *msq_msg = (struct mymsg *) malloc(sizeof(struct mymsg));
    MCHECK(msq_msg);

    for (i = 0; i < dir_list->index; i++) {
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "<Client-%zu> Not created", i);
            errExit("fork failed: ");
        }
        else if (pid == 0) {
            // i-th child
            int fd = open(dir_list->list[i], O_RDONLY);
            SYSCHECK(fd, "open");

            tot_char = count_char(fd);
            split_file(parts, fd, tot_char);

            fill_msg(msq_msg, 0, parts[3]);

            waiting = semctl(semid, SEMCHILD, GETZCNT, 0);
            if (waiting == -1) {
                errExit("semctl failed: ");
            }
            else if (waiting == (int) (dir_list->index - 1)) {
                // Figlio prediletto forse è -1 perchè conta i file in attesa
                semOp(semid, SEMCHILD, WAIT);
            }
            else {
                // Figli diseredati
                semOp(semid, SEMCHILD, SYNC);
            }

            // start sending messages
            //...
            // chiude il file
            close(fd);

            // termina
            printf("→ <Client-%zu>: Ho finito di inviare il file <%s>\n", i+1, dir_list->list[i]);
            exit(EXIT_SUCCESS);
        }
        else{
            //parent code here!

        }
    }
    close_fd(fd1);
    free_shared_memory(&shmem);
    while(wait(NULL) > 0);

    /* -------------------- */

    for (size_t indx = 0; indx < dir_list->index; indx++) {
        free(dir_list->list[indx]);
    }

    free(dir_list);
}
