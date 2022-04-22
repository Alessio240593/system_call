/** @file defines.c
 *  @brief Contiene l'implementazione delle funzioni
 *  specifiche del progetto.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

#include "defines.h"

#define MAX_PATH  512
#define MAX_FILE_SIZE 4096


char **dirList;
size_t listIndex = 0;
size_t listSize = 1;


/**
 * Controlla se string2 inizia con gli stessi caratteri di string1
 * @param string1 - stringa per il controllo
 * @param string2 - stringa da controllare
 * @return 1 - se string2 inizia con string1
 * @return 0 - se stroing2 non inizia con string1
 * @return -1 - in caso di stringhe non valide
**/
int check_string(const char* string1, char* string2)
{
    if (string1 == NULL || string2 == NULL ||
        strlen(string1) > strlen(string2))
    {
        return -1;
    }

    size_t i;
    for (i = 0; string1[i] == string2[i]; i++);
    return (i == strlen(string1)) ? 1 : 0;

}

/**
 * Controlla se il file <path> è di dimensione <= a 4096 Byte
 * @param path - percorso dove si trova il file nel filesystem
 * @return 1 - se #path è <= 4096 Byte
 * @return 0 - se #path è > 4096 Byte
 * @return -1 - in caso di path non valido
**/
int check_size(const char *path)
{
    if(path == NULL){
        return -1;
    }

    struct stat buffer;

    if((stat(path, &buffer)) == -1){
        perror("stat");
        return -1;
    }
    else if(!S_ISREG(buffer.st_mode)){
        printf("File isn't regular file!\n");
        return -1;
    }
    else {
        return buffer.st_size <= MAX_FILE_SIZE;
    }
}

int count_char(int fd)
{
    ssize_t Br;
    char buffer[MAX_FILE_SIZE];

    Br = read(fd, buffer, MAX_FILE_SIZE);
    // the fd is now reusable
    lseek(fd, 0, SEEK_SET);

    return Br - 1;
}

void sigusr1_handler(int sig)
{
    exit(EXIT_SUCCESS % sig);
}

void client_sigint_handler(int sig)
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

    if ((chdir(argv[1])) == -1){
        perror("chdir");
        exit(EXIT_SUCCESS % sig);
    }

    printf("Ciao %s, ora inizio l’invio dei file contenuti in %s",
           getenv("USER"), getenv("PWD"));
}

static int strCompare(const void *this, const void *other)
{
    return strcmp(*(const char**)this, *(const char**)other);
}

void getDirList(const char *startPath)
{
    char newPath[MAX_PATH];

    struct dirent *de;

    DIR *dp = opendir(startPath);

    if (!dp)
        return;

    while ((de = readdir(dp)) != NULL)
    {
        if (strcmp(de->d_name, ".") != 0
            && strcmp(de->d_name, "..") != 0
            && de->d_type == DT_DIR)
        {

            if (listIndex + 1 > listSize) {
                listSize *= 2;
                dirList = (char **) realloc(dirList, listSize * sizeof(char *));
            }

            dirList[listIndex] = (char *) calloc(strlen(startPath) + 1, sizeof(char));

            strcpy(dirList[listIndex], startPath);

            listIndex++;

            strcpy(newPath, startPath);

            strcat(newPath, "/");
            strcat(newPath, de->d_name);


            getDirList(newPath);
        }
    }

    closedir(dp);
}

void fixDirList(void)
{
    size_t i = 1;
    size_t t = 1;
    size_t k;


    qsort(dirList, listIndex, sizeof(const char *), strCompare);

    while (i < listIndex) {
        if (strcmp(dirList[i], dirList[t - 1]) != 0) {
            free(dirList[t]);
            dirList[t] = (char *) calloc(strlen(dirList[i]) + 1, sizeof(char));

            strcpy(dirList[t], dirList[i]);

            t++;
        }

        i++;
    }


    for (k = t + 1; k < listIndex; k++)
        free(dirList[k]);

    listIndex = t + 1;

    dirList = (char **) realloc(dirList, (listIndex) * sizeof(char *));
}
