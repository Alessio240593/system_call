#include "defines.h"
#include "err_exit.h"

int main(int argc, char *argv[]) {
    char *path;         // Percorso passato dall'utente
    int num_file;       // Numero di file da generare
    int liv;            // Numero di livelli da generare


    // Controllo dei parametri di avvio argv
    {
        // Se gli argomenti sono minori di 2 allora è sbagliata l'esecuzione
        if(argc < 2) {
            printf("\n-- Parametri errati --\n");
            printf("Per informazioni d'uso: ./tester --help\n\n");
            exit(0);
        }

        // Se l'utente imposta la flag --help mostra l'utilizzo
        if(strcmp(argv[1],"--help") == 0) {
            printf("\nPOSSIBILI UTILIZZI:\n");
            printf("\t./tester -p path_directory\n");
            printf("\t./tester -p path_directory [-n num_file]\n");
            printf("\t./tester -p path_directory [-n num_file] [-l level]\n");
            printf("\t./tester -p path_directory [-n num_file] [-l level] [-m]\n");
            
            printf("DESCRIZIONE:\n");
            printf("Il Tester ha bisogno del percorso della cartella in cui sono presenti server\n");
            printf("e client_0, genera automaticamente una cartella chiamata tester_folder con\n");
            printf("30 file di dimensione variabile (MAX 4500 byte) su 3 livelli.\n\n");
            printf("Si possono indicare 3 flag facoltative:\n");
            printf("   -n num_file: si specifica quanti file voler creare (MAX 1000).\n");
            printf("   -l level: si specifica quanti livelli voler creare (MAX 10).\n");
            printf("   -m: effettua il make del server e client_0.\n");

            exit(0);
        } else {
            // Altrimenti se il primo argomento non è --help
            // bisogna controllare che ci siano almeno 3 argomenti
            if(argc < 3) {
                printf("\n-- Parametri errati --\n");
                printf("Per informazioni d'uso: ./tester --help\n\n");
                exit(0);
            }
        }
        
        num_file = 30;      // Imposta il numero dei file a 30 di default
        liv = 3;            // Imposta il numero di livelli a 3 di default


        // Scorri tutti gli argomenti
        for(int i = 1 ; i < argc ; i++) {
            switch (i) {
                // Primo argomento
                case 1:
                    // Se non è -p allora ha sbagliato l'inserimento
                    if(strcmp(argv[i],"-p") != 0) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    }
                    break;
                

                // Secondo argomento
                case 2:
                    // Se il percorso è vuoto
                    if(strcmp(argv[i],"") == 0) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    } else {
                        // Altrimenti assegna a path il percorso della directory
                        path = argv[i];
                    }
                    break;
                

                // terzo argomento
                case 3:
                    // Se non è -n allora ha sbagliato l'inserimento
                    if(strcmp(argv[i],"-n") != 0) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    }

                    // Se è -n ma successivamente non c'è nulla
                    if((i + 1) >= argc) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    }
                    break;
                

                // Quarto argomento
                case 4:
                    // Se il numero di file non è valido
                    if((num_file = atoi(argv[i])) == 0) {
                        printf("\nBisogna inserire un numero di file corretto.\n");
                        ErrExit("atoi fallito");
                    }

                    if(num_file <= 0) {
                        printf("\nE' stato inserito un numero di file negativo.\n");
                        exit(0);
                    }

                    if(num_file > MAXFILE) {
                        printf("\nE' stato inserito un numero di file maggiore del massimo consentito (%d).\n", MAXFILE);
                        exit(0);
                    }
                    break;


                // Quinto argomento
                case 5:
                    // Se non è -l allora ha sbagliato l'inserimento
                    if(strcmp(argv[i],"-l") != 0) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    }

                    // Se è -l ma successivamente non c'è nulla
                    if((i + 1) >= argc) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    }
                    break;
                    

                // Sesto parametro
                case 6:
                    // Se il numero di livelli non è valido
                    if((liv = atoi(argv[i])) == 0) {
                        printf("\nBisogna inserire un numero di livelli corretto.\n");
                        ErrExit("atoi fallito");
                    }

                    if(liv <= 0) {
                        printf("\nE' stato inserito un numero di livelli negativo.\n");
                        exit(0);
                    }

                    if(liv > MAXDIR) {
                        printf("\nE' stato inserito un numero di livelli maggiore del massimo consentito (%d).\n", MAXDIR);
                        exit(0);
                    }
                    break;


                // Settimo argomento
                case 7:
                    // Se non è -m oppure -me allora ha sbagliato l'inserimento
                    if(strcmp(argv[i],"-m") != 0) {
                        printf("\n-- Parametri errati --\n");
                        printf("Per informazioni d'uso: ./tester --help\n\n");
                        exit(0);
                    }    
                    break;


                // Se ci sono altri argomenti in più
                default:
                    printf("\n-- Troppi argomenti --\n");
                    printf("Per informazioni d'uso: ./tester --help\n\n");
                    exit(0);
                    break;
            }
        }
    }
    

    // Si genera la cartella di test
    generaCartella(path,liv,num_file);
    
    // Se è stata impostata la flag del Makefile, lo prova eseguire
    if(argc == 8) {
        if(access("Makefile",F_OK) == 0) {        // Esiste la cartella
            // Effettua la make clean e make
            system("make clean");
            system("make");
        } else {
            printf("Non esiste il Makefile.\n\n");
        }
    }
    
}