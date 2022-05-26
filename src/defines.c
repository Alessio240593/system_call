/** @file defines.c
 *  @brief Contiene l'implementazione delle funzioni
 *  specifiche del progetto.
 */

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
        perror("stat failed: ");
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

    if ((Br = read(fd, buffer, MAX_FILE_SIZE)) == -1){
        return -2;
    }

    // the fd is now reusable
    if (lseek(fd, 0, SEEK_SET) == -1){
        return -1;
    }

    return Br;
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
        errExit("stat failed: ");
    }
    return S_ISDIR(tmp.st_mode);
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
        perror("chdir failed: ");
        ret = 1;
    }

    if ((setenv("PWD", path, 1)) == -1) {
        perror("setenv failed: ");
        ret = -1;
    }

    return ret;
}

/**
 * Divide il file fd in PARTS parti
 * @param parts - array di stringhe contenenti le PARTS parti finali
 * @param fd - file descriptor del file da leggere
 * @param tot_char - numero totale di caratteri del file fd
 * @return 0 - in caso di successo
 * @return R \ {0} - altrimenti
 */
int split_file(char** parts, int fd, size_t tot_char)
{
    size_t chunk = tot_char / PARTS;
    ssize_t Br;
    size_t reminder = tot_char % PARTS;

    if (reminder != 0)
        chunk++;

    for (size_t i = 0; i < PARTS; i++) {
        if(reminder > 0) {
            parts[i] = (char *) calloc(chunk, sizeof(char));
            MCHECK(parts[i]);

            if ((Br = read(fd, parts[i], chunk)) == -1) {
                return -1;
            }

            parts[i][Br] = '\0';
            reminder--;
        }
        else if(reminder == 0) {
            chunk--;
            //in modo tale che poi non venga più modificato il valore di chunk
            reminder = -1;
        }
    }

    // the fd is now reusable
    lseek(fd, 0, SEEK_SET);

    return 0;
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
    char new_path[PATH_MAX];
    struct dirent *de;
    // TODO controllare se max 100 file bisogna uscire
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
            if (/*si potrebbe usare strncmp*/check_string("sendme_", de->d_name) == 0 && strstr(de->d_name, "_out") == NULL) {
                if (check_size(start_path) == 0 && dirlist->index < MAX_FILE) {
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

    dirlist->size = dirlist->index;
    dirlist->list = (char **) realloc(dirlist->list, dirlist->size * sizeof(char *));

    closedir(dp);
    return 0;
}
/**
 *  Prepara il messaggio da scrivere sul file di output
 * @param part -
 * @param path -
 * @param pid -
 * @param message
 * @return
 */
 /*
char* parts_header(int part, const char *path, pid_t pid)
{
    const char *ipcs[] = {"FIFO_1", "FIFO_2", "MsgQueue", "ShdMem"};
    //char *result = (char*) calloc(sizeof(char), MAX_LEN);
    char result[MAX_LEN];

    snprintf(result, MAX_LEN, "[Parte %d del file %s, spedita dal processo %d tramite %s]\n",
             part, path, pid, ipcs[part - 1]);

    return result ;
}
*/
/**
 * Controlla se <child> ha finito di inviare le <PARTS> parti al server
 * @param matrice - matrice N x PARTS che tiene traccia dei file inviati dal client
 * @param child - identidicativo del processo figlio
 * @return 1 - in caso <child> non abbia terminato l'invio delle parti del file
 * @return 0 - in caso <child> abbia terminato l'invio delle parti del file
 */
int child_finish(int matrice[37][4], size_t child)
{
    for (size_t i = 0; i < PARTS; ++i) {
        if(matrice[child][i] != 1){
            return 1;
        }
    }
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