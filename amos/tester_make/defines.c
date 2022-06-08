#include "defines.h"
#include "err_exit.h"

const char *nome = "tester_folder";
const char *sub = "sotto_livello_";
const char *str = "ABCDEFGHILMNOPQRSTUVZWYJKX";

void generaCartella(char *path, int liv, int num) {

    // Cambia la directory di lavoro
    errno = 0;
    if(chdir(path) == -1) {
        if(errno == EACCES) {
            printf("\nManca il permesso di ricerca su uno dei componenti del percorso fornito.\n");
        }
        
        if(errno == ENAMETOOLONG) {
            printf("\nIl percorso fornito e' troppo lungo.\n");
        }
        
        if(errno == ENOTDIR) {
            printf("\nIl percorso fornito non specifica una directory.\n");
        }

        ErrExit("chdir fallito");
    }

    // Valuta l'esistenza della cartella tester_folder
    if(access(nome,F_OK) == 0) {            // Esiste la cartella
        system("rm -rf tester_folder");     // Elimina la cartella
    }

    // Ricrea la cartella
    if(mkdir(nome, S_IRWXU | S_IXUSR) == -1) {
        ErrExit("mkdir fallito");
    }

    // Si sposta dentro tester_folder
    if(chdir(nome) == -1) {
        ErrExit("chdir fallito");
    }

    // Scrive tutti i file nei sottolivelli
    scrivi(1,liv,num);


    // Torna indietro
    if(chdir("..") == -1) {
        ErrExit("chdir fallito");
    }

}

void scrivi(int start, int end, int num) {
    // Se il livello di partenza supera quello finale,
    // allora sono finiti i livelli
    if(start > end) {
        return;
    }
    
    int file = num / (end + 1 - start);     // Numero di file del livello start
    char nuovo[25] = "";                    // Buffer per il nome della sottocartella e file sendme_
    int fd, dim;                            // File descriptor, dimensione random del file

    // Se il livello è maggiore di 1 crea la sottocartella
    if(start > 1) {
        sprintf(nuovo,"%s%d", sub, start);

        // Crea la sotto cartella
        if(mkdir(nuovo, S_IRWXU | S_IXUSR) == -1) {
            ErrExit("mkdir2 fallito");
        }

        // Cambia la directory di lavoro nella sotto cartella creata
        if(chdir(nuovo) == -1) {
            ErrExit("chdir fallito");
        }
    }


    // Crea gli N file
    srand(time(NULL));
    for(int i = 0 ; i < file ; i++) {
        sprintf(nuovo, "sendme_%d", rand() % 10000);    // Nome del file sendme_
        if((fd = open(nuovo, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IXUSR)) == -1) {
            ErrExit("open1 fallito");
        }

        // Scrive un file con una dimensione dim di byte (da 0 a 4500 kB)
        dim = rand() % MAXDIM;  
        for(int j = 0 ; j < dim ; j++) {
            if(write(fd,&str[rand() % MAXCHAR], sizeof(char)) == -1) {
                printf("write2 fallito");
            }
        }

        if(close(fd) == -1) {
            ErrExit("close1 fallito");
        }

    }

    // Chiamata ricorsiva
    scrivi(start + 1, end, num - file);


    if(start > 1) {
        // Torna nella cartella precedente
        if(chdir("..") == -1) {
            ErrExit("chdir fallito");
        }
    }
}   