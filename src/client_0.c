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

char **dirList;
size_t listIndex;
size_t listSize;


void sigusr1_handler(int sig)
{
    printf("Received signal %d\n", sig);
    exit(EXIT_SUCCESS);
}

void sigint_handler(int sig)
{
    sigset_t mySet;

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
/*
    char *buffer = (char *) calloc(MAX_PATH, sizeof(char));
    if (buffer == NULL) {
        errExit("malloc ");
    }
    getcwd(buffer, MAX_PATH);
    buffer = (char *) realloc(buffer, strlen(buffer) * sizeof(char));
*/
    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s\n", getenv("USER"), getenv("PWD"));

    printf("3\n");

    listIndex = 0;
    listSize = 1;

    dirList = (char **) calloc(listSize, sizeof(char *));

    printf("2\n");

    getDirList(path);
    /// FUNZIONE DI DEBUG => NON CI SARÀ SUL PROGETTO FINALE
    dumpDirList("before.txt");

    printf("1\n");

    fixDirList();
    /// FUNZIONE DI DEBUG => NON CI SARÀ SUL PROGETTO FINALE
    dumpDirList("after.txt");

    printf("GO!!!\n");

    for (size_t indx = 0; indx < listIndex; indx++)
        free(dirList[indx]);
}

int is_dir(const char *_path)
{
    struct stat tmp;

    if ((stat(_path, &tmp)) == -1) {
        errExit("stat ");
    }
    return S_ISDIR(tmp.st_mode);
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
