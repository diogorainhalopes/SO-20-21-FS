#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"
#include <sys/time.h>
#include <pthread.h>


#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

FILE* file_in; //input file
FILE* file_out; //output file


struct timeval start, end;


char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;


int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];  
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void processInput(FILE *fp){
    char line[MAX_INPUT_SIZE];

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), fp)) {
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
                if(numTokens != 3)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'l':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
    }
}

void *applyCommands(){
    while (numberCommands > 0){

        pthread_mutex_lock(&lockCommand);
        const char* command = removeCommand();
        if (command == NULL){
            continue;
        }
        pthread_mutex_unlock(&lockCommand);

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
                switch (type) {
                    case 'f':
                        create(name, T_FILE);
                        printf("Create file: %s\n", name);
                        break;
                    case 'd':
                        create(name, T_DIRECTORY);
                        printf("Create directory: %s\n", name);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                searchResult = lookup(name, LOOK);
                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);
                break;
            case 'd':
                delete(name);
                printf("Delete: %s\n", name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}



int main(int argc, char* argv[]){
    
    file_in = fopen(argv[1], MODE_FILE_READ);//opens inputfile
    file_out = fopen(argv[2], MODE_FILE_WRITE);//opens outputfile
    int i;

    if(argc != 4){
        errorParse();
    }

    if (file_in == NULL){
        perror(argv[1]);
        exit(1);
    }

    if (file_out == NULL){
        perror(argv[2]);
        exit(1);
    }

    /* init filesystem */
    init_fs();
    int numThreads = atoi(argv[3]);
    //printf("\n%d\n", numThreads);
    pthread_t tid[numThreads];

    /* process input and print tree */
    processInput(file_in);
    fclose(file_in);

    gettimeofday(&start, NULL);

    for (i = 0; i < numThreads; i++){
        pthread_create(&tid[i], NULL, applyCommands, NULL);
    }

    //applyCommands();
    
    for (i = 0; i < numThreads; i++){
        pthread_join(tid[i], NULL);
    } 

    print_tecnicofs_tree(file_out);
    fclose(file_out);

    /* release allocated memory */
    destroy_fs();
    pthread_mutex_destroy(&lockCommand);
    gettimeofday(&end, NULL);

    /* end timer */
    double timeReal = (end.tv_sec + end.tv_usec / 1000000.0) -
        (start.tv_sec + start.tv_usec / 1000000.0);
    printf("TecnicoFS completed in %.4f seconds.\n", timeReal);


    exit(EXIT_SUCCESS);
}
 
