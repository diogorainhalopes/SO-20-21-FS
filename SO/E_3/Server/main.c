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

int sockfd;

typedef struct rs_args{

    socklen_t addrlen;
    struct sockaddr_un client_addr;
} rs_args;

FILE* file_out;


int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}


/* Simple error parsing
 * 
 */
void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

/* Removes a command from the command list and sends it to be executed
 * Output:
 * - carry: command to be executed in applyComands
 */
char* removeCommand(int sockfd, rs_args *args) {
    char *carry;
    int c;
    args->addrlen=sizeof(struct sockaddr_un);
    char in_buffer[MAX_INPUT_SIZE];
    c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
		 (struct sockaddr *)&args->client_addr, &args->addrlen);
    if (c <= 0) errorParse();
    //Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
    in_buffer[c]='\0';
    //printf("Recebeu operação de %s: %s\n", client_addr.sun_path, in_buffer);
    carry = strdup(in_buffer);
    return carry;
}


/* Funtion to be picked up by each thread to execute commands
 */
void *applyCommands(){
    while (1){
        
        rs_args *args = (rs_args *)malloc(sizeof(rs_args)+1);
        char *command = removeCommand(sockfd, args);
        int out_buffer;// = (int *)malloc(sizeof(int));
        char token, type[MAX_INPUT_SIZE];
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %s", &token, name, type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            free(command);
            exit(EXIT_FAILURE);
        }
        switch (token) {
            case 'c':
                switch (*type) {
                    case 'f':
                        
                        printf("Create file: %s\n", name);
                        out_buffer = create(name, T_FILE);
                        sendto(sockfd, &out_buffer,  sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                        
                        break;
                    case 'd':
                        
                        printf("Create directory: %s\n", name);
                        out_buffer = create(name, T_DIRECTORY);
                        sendto(sockfd, &out_buffer,  sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                        
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                
                out_buffer = lookup(name);
                if (out_buffer >= 0)
                    printf("Search: %s found\n", name);
                else {
                    printf("Search: %s not found\n", name);
                }
                sendto(sockfd, &out_buffer,  sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                
                break;
            case 'd':
            
                printf("Delete: %s\n", name);
                out_buffer = delete(name);
                sendto(sockfd, &out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);

                break;
            case 'm':
              
                printf("Move: %s to %s\n", name, type);
                out_buffer = move(name, type);
                sendto(sockfd, &out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
            
                break;
            case 'p':
             
                printf("Print to: %s\n", name);
                file_out = fopen(name, MODE_FILE_WRITE);
                if (file_out == NULL) {
                    printf("Erro: unable to open file");
                    out_buffer = FAIL;
                }
                else {
                    print_tecnicofs_tree(file_out);
                    out_buffer = SUCCESS; 
                    fclose(file_out);
                }
                sendto(sockfd, &out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
           
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

    struct sockaddr_un server_addr;
    char *path;
    rs_args args;
    
    if (argc > 4) exit(EXIT_FAILURE);

    if(argc == 4) {
        sleep(atoi(argv[3]));
    }
    

    init_fs();

    int numThreads = atoi(argv[1]);
    pthread_t tid[numThreads];

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open socket");
        exit(EXIT_FAILURE);
    }

    path = argv[2];

    unlink(path);

    args.addrlen = setSockAddrUn(argv[2], &server_addr);
    if (bind(sockfd, (struct sockaddr *) &server_addr, args.addrlen) < 0) {
        perror("server: bind error");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < numThreads; i++){
        if(pthread_create(&tid[i], NULL, applyCommands, NULL) < 0) {
            printf("erro na criacção de tarefas");
            exit(EXIT_FAILURE);
        }

    }
  
    for (int i = 0; i < numThreads; i++){
        if(pthread_join(tid[i], NULL)< 0) {
            printf("erro na criacção de tarefas");
            exit(EXIT_FAILURE);
        }
    } 

    exit(EXIT_SUCCESS);
}


 
