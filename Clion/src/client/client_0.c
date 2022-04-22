/** @file client.c
 * @brief Contiene l'implementazione del client.
*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "defines.h"

int main(int argc, char *argv[])
{
    if (argc != 2){
        fprintf(stderr, "Usage %s <path>", argv[0]);
        exit(EXIT_FAILURE);
    }

    sigset_t mySet;

    if ((sigemptyset(&mySet))== -1){
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }

    if ((sigaddset(&mySet, SIGINT)) == -1
        || (sigaddset(&mySet, SIGUSR1)) == -1)
    {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }


    if ((sigprocmask(SIG_UNBLOCK, &mySet, NULL)) == -1){
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    if ((signal(SIGINT, client_sigint_handler)) == SIG_ERR
        || (signal(SIGUSR1, sigusr1_handler)) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    /* wait for a signal */
    pause();
    
    return 0;
}
