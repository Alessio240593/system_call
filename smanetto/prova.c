#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_PATH 4096

char **dirList;
size_t listIndex;
size_t listSize;


int strCompare(const void *this, const void *other);
void getDirList(const char *startPath);
void fixDirList(void);
void dumpDirList(const char *filename);


int main(void) {

	char *path = "/home/lotation";

	printf("3\n");

    listIndex = 0;
    listSize = 1;

    dirList = (char **) calloc(listSize, sizeof(char *));

    printf("2\n");

    getDirList(path);
    dumpDirList("before.txt");

    printf("1\n");

    fixDirList();
    dumpDirList("after.txt");

    printf("GO!!!\n");

    for (size_t indx = 0; indx < listIndex; indx++) {
		free(dirList[indx]);
	}

	return 0;
}

int strCompare(const void *this, const void *other)
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
            && strcmp(de->d_name, "..") != 0)
            //&& de->d_type == DT_DIR)
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

void dumpDirList(const char *filename)
{
    FILE *fp;
    size_t i;

    fp = fopen(filename, "w");// fopen?! is legal??
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < listIndex; i++)
        fprintf(fp, "%s\n", dirList[i]);


    fclose(fp);
}
