#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <unistd.h>

#define PROC_PATH "/proc"
#define STAT "stat"

void printHeader(){
    printf("%6s|%20s|%30s|%6s|\n", "PID", "User", "PROCNAME", "Estado");
        printf("%6s|%20s|%30s|%6s|\n",  "------", "--------------------", "------------------------------",
                                        "------");
}

void processOutput(char *s1, char *s2, char *s3, int c){
    printf("%6s|%20s|%30s|%6c|\n",  s1, s2, s3, c);
}

int *trashInt;

void repaintTop(struct timeval *t, fd_set *s){
  t->tv_sec = 1;
  t->tv_usec = 0;
  FD_ZERO(s);
  FD_SET(STDIN_FILENO, s);
}

char *pathBuilder(struct dirent* readingProcs){
    char *path = malloc (255 * sizeof(char));
    strcpy(path, PROC_PATH);
    strcat(path, "/");
    strcat(path, readingProcs->d_name);
    strcat(path, "/");
    strcat(path, STAT);
    return path;
}

int main(int argc, char *argv[]){

    FILE *procStatFile;
    char procName[BUFSIZ], procState, signalIn[255], *signalToken;
    long int avoidDirLong;
    struct stat fileStat;
    struct passwd *pwd;
    int signalCode;
    struct timeval timeToRepaint;
    fd_set input;

    trashInt = (int*) malloc(sizeof(int));

    repaintTop(&timeToRepaint, &input);

    DIR *dir;

    while(1){

        /** 
        * Leitura e exibição dos processos.
        **/

        printf("\e[2J\e[H\e[3J");

        dir = opendir(PROC_PATH);
    
        printHeader();
    
        struct dirent* readingProcs;

        while((readingProcs = readdir(dir))){
        avoidDirLong = strtol(readingProcs->d_name, NULL, 10);
        
        if(avoidDirLong != 0L){
            char *path = pathBuilder(readingProcs);
            stat(path, &fileStat);
            if((pwd = getpwuid(fileStat.st_uid)) != NULL){
                procStatFile = fopen(path, "r");
                fscanf(procStatFile, "%d", trashInt);
                fscanf(procStatFile, "%s", procName);
                fscanf(procStatFile, " %c", &procState);
                processOutput(readingProcs->d_name, pwd->pw_name, procName + 1, procState);
                fclose(procStatFile);
            }
        }
        }

        closedir(dir);

    int slct = select(FD_SETSIZE, &input, NULL, NULL, &timeToRepaint);
    
    if(slct >= 0){
        fgets(signalIn, sizeof(signalIn), stdin);
        signalToken = strtok(signalIn, " \n");
      
        avoidDirLong = strtol(signalToken, NULL, 10);
        if(avoidDirLong != 0L){

            int id = avoidDirLong;
            signalToken = strtok(NULL, " \n");
            avoidDirLong = strtol(signalToken, NULL, 10);

            if(avoidDirLong != 0L){
                signalCode = avoidDirLong;
                signalToken = strtok(NULL, " \n");
                if(signalToken == NULL){
                    kill(id, signalCode);
                }
            }
        }
    }
    repaintTop(&timeToRepaint, &input);
  }

    free(trashInt);
    exit(EXIT_SUCCESS);
}