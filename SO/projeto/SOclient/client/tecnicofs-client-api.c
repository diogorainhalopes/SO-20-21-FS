#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>


int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;
char buffer[1000];
char *server;


void dg_cli(int sockfd, char* out_buffer, int c) {
    bzero((char *)buffer, sizeof(buffer));
    if (sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
        printf("client: sendto error, unable to connect to %s\n", serv_addr.sun_path);
        exit(EXIT_FAILURE);
    }
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
        printf("client: recvfrom error, unable to connect to %s\n", serv_addr.sun_path);
        exit(EXIT_FAILURE);
    }
}



int setSockAddrUn(char *path, struct sockaddr_un *addr) {

    if (addr == NULL) {
      printf("addr null\n");
      return 0;
    }

    bzero((char *)addr, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    return SUN_LEN(addr);
}

  

int tfsCreate(char *filename, char nodeType) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "c %s %c", filename, nodeType);
    dg_cli(sockfd, out_buffer, c);
    return atoi(buffer);
}

int tfsDelete(char *path) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "d %s", path);
    dg_cli(sockfd, out_buffer, c);
    return atoi(buffer);
}

int tfsMove(char *from, char *to) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "m %s %s", from, to);
    dg_cli(sockfd, out_buffer, c);
    return atoi(buffer);
}

int tfsLookup(char *path) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "l %s", path);
    dg_cli(sockfd, out_buffer, c);
    return atoi(buffer);
}

int tfsPrint(char *path) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "p %s", path);
    dg_cli(sockfd, out_buffer, c);
    return atoi(buffer);
}

int tfsMount(char * sockPath) {
    char client_name[1024];
    sprintf(client_name, "%s%d", "/tmp/client", getpid());

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        printf("client: can't open socket %d\n", sockfd);  
        return -1;
    }
    unlink(client_name); 
    clilen = setSockAddrUn(client_name, &client_addr); 
    /*
    bzero((char *) &client_addr, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, client_name);
    clilen = sizeof(client_addr.sun_family) + strlen(client_addr.sun_path);
*/
    if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
        printf("client: bind error\n");
        return -1;
    }
/*
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, sockPath);
    servlen = sizeof(serv_addr.sun_family) + strlen(serv_addr.sun_path);
*/
    servlen = setSockAddrUn(sockPath, &serv_addr);  
    if (servlen == 0) {
        printf("wrong server address");
        return -1;
    }
    return 0;
}

int tfsUnmount(char* argv[]) {
  close(sockfd);
  //unlink(argv[2]);
  return -1;
}



