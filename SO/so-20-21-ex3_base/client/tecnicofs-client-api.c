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

#define CLIENT_ADDR "/tmp/clientAdress"



int setSockAddrUn(char *path, struct sockaddr_un *addr) {

  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

  int sockfd;
  socklen_t servlen, clilen;
  struct sockaddr_un serv_addr, client_addr;
  char buffer[1024];
  char* server;


int tfsCreate(char *filename, char nodeType) {

if (sendto(sockfd, "MENSAGEM", strlen("MENSAGEM")+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
    perror("client: sendto error");
    exit(EXIT_FAILURE);
  }

if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
    perror("client: recvfrom error");
    exit(EXIT_FAILURE);
  }
 printf("Recebeu resposta do servidor: %s\n", buffer);
  return -1;
}

int tfsDelete(char *path) {
  return -1;
}

int tfsMove(char *from, char *to) {
  return -1;
}

int tfsLookup(char *path) {
  return -1;
}

int tfsMount(char * sockPath) {
server = sockPath;
printf("server name:%s\n", server);
if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    perror("client: can't open socket");  
    return -1;
  }


unlink(CLIENT_ADDR);

  clilen = setSockAddrUn (CLIENT_ADDR, &client_addr);
if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
    perror("client: bind error");
    return -1;
  }

servlen = setSockAddrUn(server, &serv_addr);  

  return 0;
}

int tfsUnmount(char* argv[]) {
  close(sockfd);
  unlink(argv[2]);
  return -1;
}
