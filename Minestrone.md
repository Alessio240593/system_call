# Client
---

1. **Create signals handlers**
``` c
#include <signal.h>
#include <sys/stat.h>
#include <string.h>

/**
*Controlla se string2 inizia con gli stessi caratteri di string1
*@param string1 - stringa per il controllo
*@param string2 - stringa da controllare
*@return 1 - se string2 inizia con string1
*@return 0 - se stroing2 non inizia con string1
*@return -1 - in caso di stringhe non valide
**/
int check_string(const char* string1, char* string2){
	
	if(string1 == NULL || string2 == NULL || len(string1) > len(string2){
		return -1
	}

	int i;	
	for (i = 0; string1[i] == string2[i]; i++);
	return (i == strlen(string1)) ? 1 : 0;
   
}

  
/**
*Controlla se il file <path> è di dimensione <= a 4096 Byte
*@param path - percorso dove si trova il file nel filesystem
*@return 1 - se #path è <= 4096 Byte
*@return 0 - se #path è > 4096 Byte
*@return -1 - in caso di path non valido
**/
int check_size(const char *path){

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
		return buffer.st_size <= 4096;
	}
}

  

void sigusr1_handler(int sig) {
	exit(0);
}

  

void sigint_handler(int sig) {

	sigset_t mySet;

	if((sigfillset(&mySet)) == -1){
		perror("sigfillset");
		exit(1);//???
	} 

	if((sigprocmask(SIG_SETMASK, &mySet, NULL)) == -1){
		perror("sigprocmask");
		exit(1);//???
	}

	if((chdir(argv[1])) == -1){
		perror("chdir");
		exit(1);//???
	}

	printf("Ciao %s, ora inizio l’invio dei file contenutiin %s",  getenv("USER"), getenv("PWD"))//check?

}

```
1. **Create and set mask**
``` c
int main(int argc, char *argv[], char *envp[]){
	if(argc < 2){
		printf("Use ./Client_0 <path>");
		exit(1);
	}
	else if(argc > 2){
		printf("Too many arguments")
	}
	
	
	sigset_t mySet;
	
	if((sigemptyset(&mySet))== -1){
		perror("sigemptyset");
		exit(1);//???
	}
	
	if((sigaddset(&mySet, SIGINT)) || (sigaddset(&mySet, SIGUSR1))) == -1){
		perror("sigaddset");
		exit(1);//???
	}
	
	
	if((sigprocmask(SIG_UNBLOCK, &mySet, NULL)) == -1){
		perror("sigprocmask");
		exit(1);//???
	}
	
	if((signal(SIGINT, sigint_handler)) || (signal(SIGUSR1, sigusr1_handler)) == SIG_ERR){
		perror("signal");
		exit(1);//???
	}
	pause() //wait for a signal	
}
		
``` 

1. **Iterate over directories to find files**
```c

/*
--------------------------------------------------------------------------------
 WEB METHOD
--------------------------------------------------------------------------------*/



void sprint(char *filename, char * dirToOpen, int level) {

	DIR *dir;
	struct dirent *entry;
	struct stat s;

	if (!(dir = opendir(dirToOpen)))
		return;

	if (!(entry = readdir(dir)))
		return;

	do {

		if(lstat(dirToOpen, &s) == 0 && S_ISDIR(s.st_mode)) /*if it's a directory*/ {
			char path[1024];
			int len = snprintf(path, sizeof(path)-1, "%s/%s", dirToOpen, entry->d_name); /*makes pathname*/
			path[len] = 0;

			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) /*if the directory isn't . or ..*/
				continue;

			printf("%*s %c → %s\n", level * 2, "", entry->d_type == DT_REG ? 'F' : 'D' , entry->d_name);
			sprint(filename ,path, level + 1);
			}
			else {

				if(strcmp(entry->d_name, filename) == 0 || strcmp(filename, ".") == 0) /*if entry name corresponds to filename, print it*/
					printf("%*s %c → %s\n", 2, "", entry->d_type == DT_REG ? 'F' : 'D' , entry->d_name);

			}

	} while (entry = readdir(dir)); /*while there are more entries to read*/

	closedir(dir);

}


/*
--------------------------------------------------------------------------------
 MIK METHOD
--------------------------------------------------------------------------------*/



#define MAX_PATH    512

char **dirList;
size_t listIndex = 0;
size_t listSize = 1;


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


    qsort(dirList, listIndex, sizeof(const char*), strCompare);

    while (i < listIndex) {
        if (strcmp(dirList[i], dirList[t - 1]) != 0) {
            free(dirList[t]);
            dirList[t] = (char *) calloc(strlen(dirList[i]) + 1, sizeof(char));

            strcpy(dirList[t], dirList[i]);

            t++;
        }

        i++;
    }


    for (k = t+1; k < listIndex; k++)
        free(dirList[k]);

    listIndex = t + 1;

    dirList = (char **) realloc(dirList, (listIndex) * sizeof(char *));
}
```
	
	
---
---


	
# SERVER
---

1.**Create the IPCS**

```c
	#define MAX_LEN 100
	#define SHMSEM 0
	
	//insert this structure in a header file <sem.h>
	struct sembuf { 
		unsigned short sem_num; /* Semaphore number */ 
		short sem_op; /* Operation to be performed */ 
		short sem_flg; /* Operation flags */ 
	};

	char* buffer[MAX_LEN];
	int n;
	struct sembuf = {.sem_num = 0, .sem_op = -1, sem_flg = 0};
	char* path1 = /home/gino/fifo1
	char* path2 = /home/gino/fifo2


	void sigint_handler(int sig) {
		//remove a semSet
		if (semctl(semid, 0/*ignored*/, IPC_RMID, 0/*ignored*/) == -1){
			perror("semctl failed");
		}
		else{
			printf("<Server>: semaphore set removed successfully\n");
		}
		
		//remove messange queue
		if (msgctl(msqid, IPC_RMID, NULL) == -1){
			perror("msgctl failed");
		}
		else{
			printf("<Server>: message queue removed successfully\n");
		}
		
		//detach the shared memory segments
		if (shmdt(ptr_1) == -1){
			perror("shmdt failed);
		}
		
		//""remove"" shared memory !warning!
		if (shmctl(shmid, IPC_RMID, NULL) == -1){
			perror("shmctl failed"); 
		else {
			printf("<Server>: shared memory segment removed successfully\n");
		}
			
		//close 2 fifo
		if(((close(fifofd1)) || (close(fifofd2))) == -1){
			perror("close");
		}
		
		//remove fifo1
		if((unlink(path1) == -1){
			perror("unlink");
		}
		else{
			printf("<Server>: fifo1 removed successfully\n");
		}
		
		//remove fifo2
		if(unlink(path2) == -1){
			perror("unlink");
		}
		else{
			printf("<Server>: fifo2 removed successfully\n");
		}
			
		exit(0);
	}

	
	int main(int argc, char* argv[], char* envp[]){
		if(signal(SIGINT, sigint_handler) == SIG_ERR){
			perror("signal");
			exit(1);
		}	
	
		//create 2 FIFO
		if((mkfifo(path1, S_IRUSR | S_IWUSR) == -1 || (mkfifo(path2, S_IRUSR | S_IWUSR) == -1){
			perror("mkfifo");
			exit(1);
		}

		//create shared memory
		if((int shmid = shmget(key_t key, size_t size, int shmflg)) == -1){
			perror("shmget:");
			exit(1);
		}

		//create messange queue
		if((int msgid = msgget(key_t key, int msgflg)) == -1){
			perror("msgget:");
			exit(1);
		}

		//create semSet of 1 sem (shared memory)
		if((int semid = semget(key_t key, int nsems, int semflg) == -1)){
			perror("semget:");
			exit(1);
		}

		//open fifo in read mode and wait until client process open fifo in write mode (this call blocks server process)
		if((int fifofd1 = open(path1, RDONLY)) == -1){
			perror("open:");
			exit(1);
		}

		if((int fifofd2 = open(path1, RDONLY)) == -1){
			perror("open:");
			exit(1);
		}

		//read blocks this process until other process write data on FIFO1
		if((int Br = read(fifofd, n, sizeof(int)) == -1){
			perror("read:");
			exit(1);
		}

		if((semOp()))
	}
	
```
	





