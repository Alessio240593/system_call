/** @file client.c
 * @brief Contiene l'implementazione del client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/stat.h>

#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "segnali.h"
#include "defines.h"
#include "err_exit.h"

const char *path;
const char *signame[]={"INVALID", "SIGHUP", "SIGINT", "SIGQUIT",
                       "SIGILL","SIGTRAP", "SIGABRT", "SIGBUS",
                       "SIGFPE", "SIGKILL","SIGUSR1", "SIGSEGV",
                       "SIGUSR2", "SIGPIPE", "SIGALRM","SIGTERM",
                       "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP",
                       "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG",
                       "SIGXCPU","SIGXFSZ", "SIGVTALRM", "SIGPROF",
                       "SIGWINCH", "SIGPOLL","SIGPWR", "SIGSYS", NULL};

void sigusr1_handler(int sig)
{
    printf(" → Received signal %s\n", signame[sig]);
    exit(EXIT_SUCCESS);
}

void sigint_handler(int sig)
{
    //declare sigset
    sigset_t mySet;

    //notify signal
    printf(" → Received signal %s\n", signame[sig]);

    //fill sigset
    if ((sigfillset(&mySet)) == -1) {
        perror("sigfillset");
        exit(EXIT_FAILURE);
    }

    //set signal mask
    if ((sigprocmask(SIG_SETMASK, &mySet, NULL)) == -1) {
        perror("sigprocmask");
        fprintf(stderr, "Received signal %d\n", sig);
        exit(EXIT_FAILURE);
    }

    //change working directory
    Chdir(path);

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n", getenv("USER"), getenv("PWD"));

    /* dirlist declaration and initialization */
    dirlist_t *dir_list = (dirlist_t *) malloc(sizeof(dirlist_t));
    MCHECK_V(dir_list);

    dir_list->index = 0;
    dir_list->size  = 1;
    dir_list->list = (char **) calloc(dir_list->size, sizeof(char *));
    MCHECK_V(dir_list->list);

    init_dirlist(dir_list, path);
    dump_dirlist(dir_list, "before.txt");


    char buffer[LEN_INT];

    snprintf(buffer, LEN_INT, "%zu", dir_list->size);

    /*
    printf("Sto per creare la fifo\n");
    make_fifo(FIFO1);
    printf("Ho creato la fifo\n");
    */

    //open fifo in write only mode
    int fd1 = open(FIFO1, O_WRONLY);
    SYSCHECK_V(fd1, "open");

    //write on fifo
    ssize_t bW = write(fd1, buffer, LEN_INT);
    WCHECK_V(bW, LEN_INT);

    //get server shmem
    int shmid = get_shared_memory(SHMKEY, SHMSIZE);
    char *shmem = (char *)attach_shared_memory(shmid, 0);

    //get server semset
    int semid = get_semaphore(SEMKEY, SEMNUM);

    //waiting data
    semOp(semid,0,-1);

    //print shmem data
    printf("%s", shmem);

    /* -------------------- */

    for (size_t indx = 0; indx < dir_list->index; indx++) {
        free(dir_list->list[indx]);
    }

    free(dir_list);
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
    if ((sigfillset(&mySet)) == -1) {
        errExit("sigemptyset");
    }

    //delete SIGINT and SIGUSR1 from the set
    if ((sigdelset(&mySet, SIGINT)) == -1
        || (sigdelset(&mySet, SIGUSR1)) == -1) {
        errExit("sigdelset");
    }

    //set the mask
    if ((sigprocmask(SIG_SETMASK, &mySet, NULL)) == -1) {
        errExit("sigprocmask");
    }

    //set the default handler
    if ((signal(SIGINT, sigint_handler)) == SIG_ERR
        || (signal(SIGUSR1, sigusr1_handler)) == SIG_ERR) {
        errExit("signal");
    }

    //wait for a signal
    pause();
}
