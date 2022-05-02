#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH 4096

#define MCHECK(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return 1;       \
                     }

#define MCHECK_V(ptr)  if (ptr == NULL) { \
                        perror("malloc "); \
                        return;       \
                     }


typedef struct __dirlist_t {
    char **list;
    size_t index;
    size_t size;
} dirlist_t;

int check_size(const char *path);
int check_string(const char *string1, char *string2);
int str_compare(const void *this, const void *other);
int init_dirlist(dirlist_t *dirlist, const char *start_path);
int  fix_dirlist(dirlist_t *dirlist);
int dump_dirlist(dirlist_t *dirlist, const char *filename);


int main(void)
{
    /* Start path */
    char *path = "/tmp";

    /* dirlist declaration and initialization */
    dirlist_t *dir_list = (dirlist_t *) malloc(sizeof(dirlist_t));
    MCHECK(dir_list);

    dir_list->index = 0;
    dir_list->size  = 1;
    dir_list->list = (char **) calloc(dir_list->size, sizeof(char *));
    MCHECK(dir_list->list);

    init_dirlist(dir_list, path);
    dump_dirlist(dir_list, "before.txt");


    fix_dirlist(dir_list);
    dump_dirlist(dir_list, "after.txt");


    for (size_t indx = 0; indx < dir_list->index; indx++) {
        free(dir_list->list[indx]);
    }

    return 0;
}

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
        return buffer.st_size > MAX_PATH;
    }
}

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

int str_compare(const void *this, const void *other)
{
    return strcmp(*(const char**)this, *(const char**)other);
}

int init_dirlist(dirlist_t *dirlist, const char *start_path) {
    char new_path[MAX_PATH];

    struct dirent *de;

    DIR *dp = opendir(start_path);

    if (!dp) {
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
            if (check_string(de->d_name, "sendme_") == 0) {
                if (check_size(dirlist->list[dirlist->index]) == 0) {
                    if (dirlist->index + 1 > dirlist->size) {
                        dirlist->size *= 2;
                        dirlist->list = (char **) realloc(dirlist->list, dirlist->size * sizeof(char *));
                        MCHECK(dirlist->list);
                    }

                    dirlist->list[dirlist->index] = (char *) calloc(strlen(start_path) + 1, sizeof(char));
                    MCHECK(dirlist->list[dirlist->index]);

                    strcpy(dirlist->list[dirlist->index], start_path);
                    dirlist->index++;
                }
            }
        }
    }

    closedir(dp);

    return 0;
}

int fix_dirlist(dirlist_t *dirlist)
{
    size_t i = 1;
    size_t t = 1;
    size_t k;

    qsort(dirlist->list, dirlist->index, sizeof(const char *), str_compare);

    while (i < dirlist->index) {
        if (strcmp(dirlist->list[i], dirlist->list[t - 1]) != 0) {
            free(dirlist->list[t]);

            dirlist->list[t] = strdup(dirlist->list[i]);

            t++;
        }

        i++;
    }


    for (k = t + 1; k < dirlist->index; k++) {
        free(dirlist->list[k]);
    }

    dirlist->index = t + 1;

    dirlist->list = (char **) realloc(dirlist->list, (dirlist->index) * sizeof(char *));
    MCHECK(dirlist->list);

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
