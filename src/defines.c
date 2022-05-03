/** @file defines.c
 *  @brief Contiene l'implementazione delle funzioni
 *  specifiche del progetto.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "defines.h"
#include "err_exit.h"

/**
 * Controlla se string2 inizia con gli stessi caratteri di string1
 * @param string1 - stringa per il controllo
 * @param string2 - stringa da controllare
 * @return  1 - se stroing2 non inizia con string1
 * @return  0 - se string2 inizia con string1
 * @return -1 - in caso di stringhe non valide
**/
int check_string(const char *string1, char *string2)
{
    if (string1 == NULL || string2 == NULL ||
        strlen(string1) > strlen(string2)) {
        return -1;
    }

    size_t i;
    for (i = 0; string1[i] == string2[i]; i++);

    return (i == strlen(string1)) ? 0 : 1;
}

/**
 * Controlla se il file <path> è di dimensione <= a 4096 Byte
 * @param path - percorso dove si trova il file nel filesystem
 * @return  1 - se #path è > 4096 Byte
 * @return  0 - se #path è <= 4096 Byte
 * @return -1 - in caso di path non valido
**/
int check_size(const char *path)
{
    if (path == NULL) {
        return -1;
    }

    struct stat buffer;

    if ((stat(path, &buffer)) == -1) {
        perror("stat");
        return -1;
    }
    else {
        return buffer.st_size > MAX_FILE_SIZE;
    }
}

/**
 * Conta il numero di caratteri del file passato come parametro
 * @param fd - file descriptor del file
 * @return Br - numero di caratteri del file
 * @return -1 - in caso di errore delle system call
**/
ssize_t count_char(int fd)
{
    ssize_t Br;
    char buffer[MAX_FILE_SIZE + 1];

    if((Br = read(fd, buffer, MAX_FILE_SIZE)) == -1){
        return -1;
    }

    // the fd is now reusable
    if(lseek(fd, 0, SEEK_SET) == -1){
        return -1;
    }

    return Br - 1;
}

/**
 * Cambia la working directory con <path>, aggiorna la variabile d'ambiente "PWD" con il nuovo valore
 * Assume che <path> sia regolare.
 * @param path - nuova working directory
**/
int Chdir(const char *path)
{
    int ret = 0;

    if ((chdir(path)) != 0) {
        perror("chdir");
        ret = 1;
    }

    if ((setenv("PWD", path, 1)) == -1) {
        perror("setenv");
        ret = -1;
    }

    return ret;
}

/**
 * Controlla se <path> è una directory
 * @param path - path da controllare
 * @return 0 - in caso non sia una directory
 * @return R \ {0} - se è una directory
**/
int is_dir(const char *path)
{
    struct stat tmp;

    if ((stat(path, &tmp)) == -1) {
        errExit("stat ");
    }
    return S_ISDIR(tmp.st_mode);
}

/**
 * Ricerca ricorsivamente i file che iniziano con sendme_ e hanno # inferiore a 4K
 * @param dirlist - struttura contenente i percorsi dei file
 * @param start_path - percorso di partenza della ricerca
 * @return 0 - in caso di successo
 * @return 1 - in caso di errore di allocazione della memoria
 * @return 2 - in caso la system call opendir fallisca
**/
int init_dirlist(dirlist_t *dirlist, const char *start_path) {
    char new_path[MAX_PATH];
    struct dirent *de;

    DIR *dp = opendir(start_path);

    if (!dp) {
        fprintf(stderr, "opendir: unable to open directory %s\n", start_path);
        return 2;
    }

    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0
            || strcmp(de->d_name, "..") == 0)
        { continue; }

        if (de->d_type == DT_DIR) {
            strcpy(new_path, start_path);
            strcat(new_path, "/");
            strcat(new_path, de->d_name);
            strcat(new_path, "\0");

            init_dirlist(dirlist, new_path);
        }
        else if (de->d_type == DT_REG) {
            if (check_string("sendme_", de->d_name) == 0) {
                if (check_size(start_path) == 0) {
                    if (dirlist->index + 1 > dirlist->size) {
                        dirlist->size *= 2;
                        dirlist->list = (char **) realloc(dirlist->list, dirlist->size * sizeof(char *));
                        MCHECK(dirlist->list);
                    }

                    size_t to_alloc =  strlen(start_path) + strlen(de->d_name) + 1 + 1 + 1;
                    dirlist->list[dirlist->index] = (char *) calloc(to_alloc , sizeof(char));
                    MCHECK(dirlist->list[dirlist->index]);

                    snprintf(dirlist->list[dirlist->index], to_alloc, "%s/%s", start_path, de->d_name);

                    dirlist->index++;
                }
            }
        }
    }

    closedir(dp);

    return 0;
}


/// FUNZIONE DI DEBUG => NON CI SARÀ SUL PROGETTO FINALE
int dump_dirlist(dirlist_t *dirlist, const char *filename)
{
    FILE *fp;
    size_t i;

    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < dirlist->index; i++) {
        fprintf(fp, "%s\n", dirlist->list[i]);
    }

    fclose(fp);

    return 0;
}

/*
char* itoa(int num)
{
    int tmp = num;
    size_t i = 0;
    char *res = (char *) calloc(10, sizeof(char));
    if (res == NULL)
        errExit("malloc");

    while (tmp > 0) {
        res[i] = tmp % 10 + 48;
        tmp /= 10;
        i++;
    }

    char *result = (char *) calloc(strlen(res) +1, sizeof(char));
    if (result == NULL)
        errExit("malloc");

    for (i = 0; i < strlen(res); i++) {
        result[i] = res[strlen(res) - i - 1];
    }
    result[i] = '\0';
}
*/