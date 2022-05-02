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

unsigned int mode =        0;
#define SC_BY_NAME         1
#define SC_BY_PERMISSIONS  2
#define SC_BY_SIZE         3
// if mode is SC_BY_NAME, then name2match is the file name we have to search for
char *name2match = "sendme_";
// if mode is SC_BY_SIZE, then size2match is the file size we have to search for
off_t size2match = ;

// the current seachPath. It is updated by search method to recursively
// traverse the filesystem
char seachPath[250];


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
    else if (!S_ISREG(buffer.st_mode)) {
        printf("File isn't regular file!\n");
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

size_t append2Path(char *directory) {
    size_t lastPathEnd = strlen(seachPath);
    // extends current seachPath: seachPath + / + directory
    strcat(strcat(&seachPath[lastPathEnd], "/"), directory);
    return lastPathEnd;
}

void search(void) {
    // open the current seachPath (open directory searchPath)
    DIR *dirp = opendir(seachPath);
    if (dirp == NULL) return;
    // readdir returns NULL when end-of-directory is reached.
    // In oder to get when an error occurres, we set errno to zero, and the we
    // call readdir. If readdir returns NULL, and errno is different from zero,
    // an error must have occurred.
    errno = 0;
    // iter. until NULL is returned as a result
    struct dirent *dentry;
    // while (readdir(dirp))
    while ((dentry = readdir(dirp)) != NULL) {
        // Skip . and ..
        if (strcmp(dentry->d_name, ".") == 0 ||
            strcmp(dentry->d_name, "..") == 0)
        {  continue;  }

        // is the current dentry a regular file?
        //printf("Si inzia a cercare:\n%d == %d ? %s\n", dentry->d_type, DT_REG, dentry->d_type == DT_REG ? "si" : "no");

        if (dentry->d_type == DT_REG) {
            // exetend current seachPath with the file name
            size_t lastPath = append2Path(dentry->d_name);
            // checking the properties of the file according to mode
            /*
            int match =
              // if mode is equal to SC_BY_NAME, then we check the file name
              (mode == SC_BY_NAME)? checkFileName(dentry->d_name, name2match)
              // if mode is equal to SC_BY_PERMISSIONS, then we check the file permissions
            : (mode == SC_BY_PERMISSIONS)? checkPermissions(seachPath, mode2match)
              // if mode is equal to SC_BY_NAME, then we check the file size
            : (mode == SC_BY_SIZE)? checkFileSize(seachPath, size2match) : 0;
            */
            //printf("dentry->d_name: %s\tname2match: %s\n", dentry->d_name, name2match);
            //printf("seachPath: %s\tmode2match: %d\n", seachPath, mode2match);
            //printf("seachPath: %s\tsize2match: %ld\n", seachPath, size2match);

            int match;
            if (mode == SC_BY_NAME)
                match = checkFileName(dentry->d_name, name2match);
            else if (mode == SC_BY_PERMISSIONS)
                match = checkPermissions(seachPath, mode2match);
            else
                match = checkFileSize(seachPath, size2match);

            // if match is 1, then a research...
            if (match == 1)
                printf("%s\n", seachPath);
            // reset current seachPath
            seachPath[lastPath] = '\0';
            // is the current dentry a directory?
        } else if (dentry->d_type == DT_DIR) {
            // exetend current seachPath with the directory name
            size_t lastPath = append2Path(dentry->d_name);
            // call search method
            search();
            // reset current seachPath
            seachPath[lastPath] = '\0';
        }
        errno = 0;
    }

    if (errno != 0)
        errExit("readdir failed");

    if (closedir(dirp) == -1)
        errExit("closedir failed");
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
