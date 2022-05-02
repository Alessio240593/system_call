/** @file client.c
 * @brief Contiene l'implementazione del client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

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
    sigset_t mySet;

    printf(" → Received signal %s\n", signame[sig]);

    if ((sigfillset(&mySet)) == -1) {
        perror("sigfillset");
        exit(EXIT_FAILURE);
    }

    if ((sigprocmask(SIG_SETMASK, &mySet, NULL)) == -1) {
        perror("sigprocmask");
        fprintf(stderr, "Received signal %d\n", sig);
        exit(EXIT_FAILURE);
    }

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

    for (size_t indx = 0; indx < dir_list->index; indx++) {
        free(dir_list->list[indx]);
    }

    free(dir_list);
}

int main(int argc, char * argv[])
{
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

    path = argv[1];

    sigset_t mySet;

    if ((sigfillset(&mySet)) == -1) {
        errExit("sigemptyset");
    }

    if ((sigdelset(&mySet, SIGINT)) == -1
        || (sigdelset(&mySet, SIGUSR1)) == -1) {
        errExit("sigdelset");
    }

    if ((sigprocmask(SIG_SETMASK, &mySet, NULL)) == -1) {
        errExit("sigprocmask");
    }

    if ((signal(SIGINT, sigint_handler)) == SIG_ERR
        || (signal(SIGUSR1, sigusr1_handler)) == SIG_ERR) {
        errExit("signal");
    }

    pause(); //wait for a signal
}
