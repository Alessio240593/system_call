/** @file defines.c
 *  @brief Contiene l'implementazione delle funzioni
 *  specifiche del progetto.
 */

#include "defines.h"
#include "err_exit.h"

/**
 * Concatena $HOME al percorso relativo di myDir e FIFO_1
 * @return - il percorso assoluto alla FIFO_1
 */
char *get_FIFO_1(void) {
    char *path = (char *) calloc(MAX_LEN, sizeof(char));
    if (!path) {
        errExit("malloc failed: ");
    }

    //snprintf(path, MAX_LEN, "%s/myDir/fifo1", getenv("HOME"));
    snprintf(path, MAX_LEN, "/tmp/fifo1");

    return path;
}

/**
 * Concatena $HOME al percorso relativo di myDir e FIFO_2
 * @return - il percorso assoluto alla FIFO_2
 */
char *get_FIFO_2(void) {
    char *path = (char *) calloc(MAX_LEN, sizeof(char));
    if (!path) {
        errExit("malloc failed: ");
    }
    
    snprintf(path, MAX_LEN, "/tmp/fifo2");

    return path;
}

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
    if(strcmp(string1, string2) == 0){
        return 0;
    }

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
    errno = 0;

    if ((stat(path, &buffer)) == -1 && errno != 0) {
        perror("stat failed: ");
        return -1;
    }
    else {
        if (buffer.st_size <= MAX_FILE_SIZE) {
            return 0;
        } else {
            return 1;
        }
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
    else
        reminder = -1;

    for (size_t i = 0; i < PARTS; i++) {
        if(reminder > 0) {
            reminder--;
        }
        else if(reminder == 0) {
            chunk--;
            //in modo tale che poi non venga piÃ¹ modificato il valore di chunk
            reminder = -1;
        }

        if (chunk != 0) {
            parts[i] = (char *) calloc(chunk, sizeof(char));
            MCHECK(parts[i]);

            if ((Br = read(fd, parts[i], chunk)) == -1) {
                return -1;
            }

            parts[i][Br] = '\0';
        }
    }

    // the fd is now reusable
    lseek(fd, 0, SEEK_SET);

    return 0;
}

/**
 * Controlla se all'interno della stringa è presente "_out"
 * @param str - stringa in input
 * @return 1 - in caso affermativo
 * @return R \ {1} - altrimenti
 */
 /*
int ends_with(const char *str, const char *end)
{
    if (!str || !end) {
        return -1;
    }

    char *token = strtok(strdup(str), ".");
    //return strncmp(str + strlen(token) - 4, "_out", 4);

    size_t out_len = strlen(end);
    int offset = strlen(token) - out_len;

    if (offset < 0) {
        return -1;
    }

    size_t i = 0;
    while (i < out_len && token[offset + i] == end[i]) {
        i += 1;
    }

    //return strncmp(str + strlen(token) - 4, "_out", 4);
    return i != out_len;
}*/

int ends_with(const char *str){
     if (!str) {
        return -1;
    }

    for (size_t i = 0; i < strlen(str); i++)
    {
        if(i < strlen(str) - 3 && str[i] ==  '_'){
            if(str[i + 1] ==  'o' && str[i + 2] ==  'u' && str[i + 3] ==  't'){
                return 0;
            }
        }
    }

    return 1;
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

            if (check_string("sendme_", de->d_name) == 0 &&
                  ends_with(de->d_name) == 1 &&
                    check_size(de->d_name) == 0 &&
                      dirlist->index < MAX_FILE)
            {
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

    dirlist->size = dirlist->index;
    dirlist->list = (char **) realloc(dirlist->list, dirlist->size * sizeof(char *));

    closedir(dp);
    return 0;
}

/**
 * Controlla se <child> ha finito di inviare le <PARTS> parti al server
 * @param matrice - matrice N x PARTS che tiene traccia dei file inviati dal client
 * @param child - identidicativo del processo figlio
 * @return 1 - in caso <child> non abbia terminato l'invio delle parti del file
 * @return 0 - in caso <child> abbia terminato l'invio delle parti del file
 */
int has_child_finished(int **matrice, size_t child)
{
    for (size_t i = 0; i < PARTS; ++i) {
        if (matrice[child][i] != 1) {
            return 1;
        }
    }
    return 0;
}

/**
 * Appende la striga "_out" alla stringa passata come parametro
 * @param string - stringa dove verrà appeso "_out"
 * @return la stringa modificata
 **/
char *append_out(const char *string)
{
    //contiene il risultato
    char result[PATH_MAX] = {0};
    //array di supporto
    char tmp[PATH_MAX] = {0};
    //conta quanti caratteri ha letto per l'estensione
    int occ = 0;
    //copio la stringa passata in result
    strncpy(result, string, strlen(string));
    int i = strlen(result) - 1;

    //caso in cui la stringa non contenga l'estensione
    if (strrchr(result, '.') == NULL || string[strlen(string) - 1] == '.') {
        return strdup(strcat(result, "_out"));
    }
        //altrimenti la stringa ha l'estensione
    else {
        //scorro finche non trovo il primo punto e salvo l'estensione in tmp
        while (result[i] != '.') {
            tmp[i] = result[i];
            i--;
            occ++;
        }

        int j = i;
        //copio il punto e copio in tmp alla posizione giusta "_out"
        tmp[i] = result[i];
        result[i++] = '_';
        result[i++] = 'o';
        result[i++] = 'u';
        result[i++] = 't';

        // copio in result l'estensione dopo "_out"
        for (int k = 0; k <= occ; k++) {
            result[i++] = tmp[j++];
        }

        return strdup(result);
    }
}

