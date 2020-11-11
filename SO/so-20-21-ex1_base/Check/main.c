#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"
#include <sys/time.h>
#include <pthread.h>


#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100


FILE* file_in; //input file
FILE* file_out; //output file

struct timeval start, end;

pthread_mutex_t mutex;

pthread_cond_t var_in;
pthread_cond_t var_out;
int numT_var;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;
int i_in = 0;

int insertCommand(char* data) {
    pthread_mutex_lock(&mutex);

    while (numberCommands == MAX_COMMANDS) pthread_cond_wait(&var_in, &mutex);

    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[i_in++], data);
        if (i_in == MAX_COMMANDS) i_in = 0;
        numberCommands++;
        pthread_cond_signal(&var_out);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

char* removeCommand() {
    pthread_mutex_lock(&mutex);
    char* carry;
    while (numberCommands == 0) pthread_cond_wait(&var_out, &mutex);
    if(numberCommands > 0){
        numberCommands--;
        carry = strdup(inputCommands[headQueue]);
        headQueue++; if (headQueue == MAX_COMMANDS) headQueue = 0;
        pthread_cond_signal(&var_in);
        pthread_mutex_unlock(&mutex);
        return carry;  
    }
    pthread_mutex_unlock(&mutex);
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
    for (int i = 0; i < numT_var; i++) insertCommand("e ");
}

void *applyCommands(){
    while (1){
        char* command = removeCommand();

        if (strcmp(command, "e ") == 0){
            free(command);
            return NULL;
        }
        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            free(command);
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
                searchResult = lookup(name);
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
        free(command);
    }
    return NULL;
}



int main(int argc, char* argv[]){
    
    
    if(argc != 4){
        errorParse();
    }

    file_in = fopen(argv[1], MODE_FILE_READ);//opens inputfile
    file_out = fopen(argv[2], MODE_FILE_WRITE);//opens outputfile
    int i;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&var_in, NULL);
    pthread_cond_init(&var_out, NULL);



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
    numT_var = numThreads;
    //printf("\n%d\n", numThreads);
    pthread_t tid[numThreads];

    /* process input and print tree */
    
    
    for (i = 0; i < numThreads; i++){
        pthread_create(&tid[i], NULL, applyCommands, NULL);
    }
    processInput(file_in);

    fclose(file_in);

    gettimeofday(&start, NULL);


    //applyCommands();
    
    for (i = 0; i < numThreads; i++){
        pthread_join(tid[i], NULL);
    } 

    gettimeofday(&end, NULL);
    /* end timer */
    double timeReal = (end.tv_sec + end.tv_usec / 1000000.0) -
        (start.tv_sec + start.tv_usec / 1000000.0);
    printf("TecnicoFS completed in %.4f seconds.\n", timeReal);

    print_tecnicofs_tree(file_out);
    fclose(file_out);

    /* release allocated memory */
    destroy_fs();
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&var_in);
    pthread_cond_destroy(&var_out);




    exit(EXIT_SUCCESS);
}
 
