/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

int main(int argc, char * argv[]) {

    if(argc < 2){
        printf("Use ./Client_0 <path>");
        exit(1);
    }
    else if(argc > 2){
        printf("Too many arguments")
    }


    sigset_t mySet;

    if((sigemptyset(&mySet))== -1){
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }

    if((sigaddset(&mySet, SIGINT)) || (sigaddset(&mySet, SIGUSR1))) == -1){
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }


    if((sigprocmask(SIG_UNBLOCK, &mySet, NULL)) == -1){
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    if((signal(SIGINT, sigint_handler)) || (signal(SIGUSR1, sigusr1_handler)) == SIG_ERR){
        perror("signal");
        exit(EXIT_FAILURE);//???
    }

    /* wait for a signal */
    pause();

    return 0;
}
