#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXDIR 10           // Numero massimo di livelli di cartelle
#define MAXFILE 1000        // Numero massimo di file sendme_
#define MAXDIM 4501         // Numero massimo di byte dei file sendme_
#define MAXCHAR 26          // Alfabeto

// Genera la cartella tester_folder e le eventuali sottocartelle
void generaCartella(char *, int, int);

// Scrive i file per ogni cartella e crea i sottolivelli
void scrivi(int, int, int);

