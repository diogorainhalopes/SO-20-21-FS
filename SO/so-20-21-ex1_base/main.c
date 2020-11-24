#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include "fs/operations.h"
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <strings.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>


#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100


FILE* file_in; /* input file */
FILE* file_out; /* output file */

struct timeval start, end; /* time struct */

pthread_mutex_t mutex;  /* global mutex used to sync the input */

/* conditional variables */
pthread_cond_t var_in;  
pthread_cond_t var_out;
int numT_var;

char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;
int insert_iter = 0;           /* iterator in the comand list */


int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

#define INDIM 30
#define OUTDIM 512




/* Given a command, the function will add it to a list
 * Input:
 *  - data: string with instructions and information to the command
 */
int insertCommand(char* data) {
    pthread_mutex_lock(&mutex);
    // circular array 
    while (numberCommands == MAX_COMMANDS) pthread_cond_wait(&var_in, &mutex);
                                        /* waits till theres memory to insert a new command */
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[insert_iter++], data);
        if (insert_iter == MAX_COMMANDS) insert_iter = 0;     // circular array 
        numberCommands++;
        pthread_cond_signal(&var_out);  /* signals that exists a command to remove */
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

/* Removes a command from the command list and sends it to be executed
 * Output:
 * - carry: command to be executed in applyComands
 */
char* removeCommand() {
    pthread_mutex_lock(&mutex);
    char* carry;
    while (numberCommands == 0) pthread_cond_wait(&var_out, &mutex);
                                /* waits till theres some command to remove */
    if(numberCommands > 0){
        numberCommands--;
        carry = strdup(inputCommands[headQueue]);
        headQueue++; if (headQueue == MAX_COMMANDS) headQueue = 0;
        pthread_cond_signal(&var_in);       /* signals that theres room to a new command */
        pthread_mutex_unlock(&mutex);
        return carry;  
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

/* Simple error parsing
 * 
 */
void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

/* Processes the input in a given file
 * Input:
 * - fp: file with commands and data to be processed
 */
void processInput(FILE *fp){
    char line[MAX_INPUT_SIZE];
    
    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), fp)) {
        char token, type[100];
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %s", &token, name, type);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':                   /* create */
                if(numTokens != 3)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'l':                   /* lookup */
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'd':                   /* delete */
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            case 'm':                   /* move */
                if(numTokens != 3)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case '#':                   /* comment */
                break;
            
            default: {                  /* error */
                errorParse();
            }
        }
    }
    for (int i = 0; i < numT_var; i++) insertCommand("e "); /* EOF command to end threads */
}

/* Funtion to be picked up by each thread to execute commands
 */
void *applyCommands(){
    while (1){
        char* command = removeCommand();

        if (strcmp(command, "e ") == 0) {    /* if the verification is successful, */
            free(command);                   /* calls to end thread */
            return NULL;
        }
        char token, type[MAX_INPUT_SIZE];
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %s", &token, name, type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            free(command);
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c':
                switch (*type) {
                    case 'f':
                        printf("Create file: %s\n", name);
                        create(name, T_FILE);
                        break;
                    case 'd':
                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);
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
                printf("Delete: %s\n", name);
                delete(name);
                break;
            case 'm':
                printf("Move: %s to %s\n", name, type);
                move(name, type);
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
    
    /*
    if(argc != 4){
        errorParse();
    }

    file_in = fopen(argv[1], MODE_FILE_READ);       
    file_out = fopen(argv[2], MODE_FILE_WRITE);     
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


    init_fs();
    int numThreads = atoi(argv[3]);
    numT_var = numThreads;
    pthread_t tid[numThreads];

    
    gettimeofday(&start, NULL);

    for (i = 0; i < numThreads; i++){
        pthread_create(&tid[i], NULL, applyCommands, NULL);
    }

    processInput(file_in);

    fclose(file_in);
    
    for (i = 0; i < numThreads; i++){
        pthread_join(tid[i], NULL);
    } 


    gettimeofday(&end, NULL);

    double timeReal = (end.tv_sec + end.tv_usec / 1000000.0) -
        (start.tv_sec + start.tv_usec / 1000000.0);
    printf("TecnicoFS completed in %.4f seconds.\n", timeReal);


    print_tecnicofs_tree(file_out);
    fclose(file_out);


    destroy_fs();
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&var_in);
    pthread_cond_destroy(&var_out);

    exit(EXIT_SUCCESS);

    */


  int sockfd;
  struct sockaddr_un server_addr;
  socklen_t addrlen;
  char *path;

  if (argc < 2)
    exit(EXIT_FAILURE);

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    perror("server: can't open socket");
    exit(EXIT_FAILURE);
  }

  path = argv[1];

  unlink(path);

  addrlen = setSockAddrUn (argv[1], &server_addr);
  if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
    perror("server: bind error");
    exit(EXIT_FAILURE);
  }
  
  while (1) {
    struct sockaddr_un client_addr;
    char in_buffer[INDIM], out_buffer[OUTDIM];
    int c;

    addrlen=sizeof(struct sockaddr_un);
    c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
		 (struct sockaddr *)&client_addr, &addrlen);
    if (c <= 0) continue;
    //Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
    in_buffer[c]='\0';
    
    printf("Recebeu mensagem de %s: %s\n", client_addr.sun_path, in_buffer);
    

    c = sprintf(out_buffer, "Ola' %s, que tal vai isso?", in_buffer);
    
    sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *)&client_addr, addrlen);

  }

  //Fechar e apagar o nome do socket, apesar deste programa 
  //nunca chegar a este ponto
  close(sockfd);
  unlink(argv[1]);
  exit(EXIT_SUCCESS);

}


 
