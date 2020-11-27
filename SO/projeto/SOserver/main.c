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
#define INDIM 30
#define OUTDIM 512



typedef struct rs_args{
    int sockfd;
    socklen_t addrlen;
    struct sockaddr_un client_addr;
} rs_args;


    

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
char* removeCommand(int sockfd, socklen_t addrlen, struct sockaddr_un client_addr) {
    char *carry;
    int c;
    addrlen=sizeof(struct sockaddr_un);
    char in_buffer[MAX_INPUT_SIZE];
    c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0,
		 (struct sockaddr *)&client_addr, &addrlen);
    if (c <= 0) errorParse();
    //Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
    in_buffer[c]='\0';
    //printf("Recebeu operação de %s: %s\n", client_addr.sun_path, in_buffer);
    carry = strdup(in_buffer);
    printf("carry: %s\n", carry);
    return carry;
}


/* Funtion to be picked up by each thread to execute commands
 */
void *applyCommands(void* s_args){
    while (1){
        rs_args *args = (rs_args*)s_args;
        char *command = removeCommand(args->sockfd, args->addrlen, args->client_addr);
        int out_buffer[1];
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
                        out_buffer[0] = create(name, T_FILE);
                        sendto(args->sockfd, out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                        break;
                    case 'd':
                        printf("Create directory: %s\n", name);
                        out_buffer[0] = create(name, T_DIRECTORY);
                        printf("antes de amndar %d\n", out_buffer[0]);
                        sendto(args->sockfd, out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                        printf("apos de mandar %d\n", out_buffer[0]);

                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                out_buffer[0] = lookup(name);
                if (out_buffer >= 0)
                    printf("Search: %s found\n", name);
                else {
                    printf("Search: %s not found\n", name);
                }
                sendto(args->sockfd, out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                break;
            case 'd':
                printf("Delete: %s\n", name);
                out_buffer[0] = delete(name);
                sendto(args->sockfd, out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
                break;
            case 'm':
                printf("Move: %s to %s\n", name, type);
                out_buffer[0] = move(name, type);
                sendto(args->sockfd, out_buffer, sizeof(out_buffer), 0, (struct sockaddr *)&args->client_addr, args->addrlen);
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
    if (argc < 3) exit(EXIT_FAILURE);

    init_fs();

    int numThreads = atoi(argv[1]);
    pthread_t tid[numThreads];

    if ((args.sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open socket");
        exit(EXIT_FAILURE);
    }

    path = argv[2];

    unlink(path);

    args.addrlen = setSockAddrUn(argv[2], &server_addr);
    if (bind(args.sockfd, (struct sockaddr *) &server_addr, args.addrlen) < 0) {
        perror("server: bind error");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < numThreads; i++){
        pthread_create(&tid[i], NULL, applyCommands, &args);
    }
  
    for (int i = 0; i < numThreads; i++){
        pthread_join(tid[i], NULL);
    } 
    
  
    exit(EXIT_SUCCESS);
}


 
