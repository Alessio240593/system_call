/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "err_exit.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sigusr1_handler(int sig)
{
    exit(EXIT_SUCCESS % sig);
}

void sigint_handler(int sig)
{
    sigset_t mySet;

    if ((sigfillset(&mySet)) == -1){
        perror("sigfillset");
        exit(EXIT_FAILURE);
    }

    if ((sigprocmask(SIG_SETMASK, &mySet, NULL)) == -1){
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    /* if ((chdir(argv[1])) == -1){
         perror("chdir");
         exit(EXIT_FAILURE % sig);
     }*/

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s",
           getenv("USER"), getenv("PWD"));
}

int main(int argc, char * argv[]) {
    if(argc < 2){
        printf("Use ./Client_0 <path>");
        exit(EXIT_FAILURE);
    }
    else if(argc > 2){
        printf("Too many arguments");
        exit(EXIT_FAILURE);
    }

    sigset_t mySet;

    if((sigemptyset(&mySet))== -1){
        errExit("sigemptyset");
    }

    if((sigaddset(&mySet, SIGINT)) || (sigaddset(&mySet, SIGUSR1)) == -1){
        errExit("sigaddset");
    }

    if((sigprocmask(SIG_UNBLOCK, &mySet, NULL)) == -1){
        errExit("sigprocmask");
    }

    if((signal(SIGINT, sigint_handler)) || (signal(SIGUSR1, sigusr1_handler)) == SIG_ERR){
        errExit("signal");
    }
    pause(); //wait for a signal
}
