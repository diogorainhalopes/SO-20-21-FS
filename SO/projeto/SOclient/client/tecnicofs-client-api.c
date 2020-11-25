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
char buffer[1024];
char* server;


int setSockAddrUn(char *path, struct sockaddr_un *addr) {

    if (addr == NULL)
      return 0;

    bzero((char *)addr, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);

    return SUN_LEN(addr);
}

  

int tfsCreate(char *filename, char nodeType) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "c %s %c", filename, nodeType);
    if (sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("client: sendto error");
        return -1;
    }
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
        perror("client: recvfrom error");
        return -1;
    }
    printf("%d\n", buffer[0]);
    return 0;
}

int tfsDelete(char *path) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "d %s", path);
    if (sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("client: sendto error");
        return -1;
    }
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
        perror("client: recvfrom error");
        return -1;
    }
    printf("%d\n", buffer[0]);
    return 0;
}

int tfsMove(char *from, char *to) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "m %s %s", from, to);

    if (sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("client: sendto error");
        return -1;
    }

    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
        perror("client: recvfrom error");
        return -1;
    }
 printf("%d\n", buffer[0]);
  return 0;
}

int tfsLookup(char *path) {
    char out_buffer[MAX_INPUT_SIZE];
    int c;
    c = sprintf(out_buffer, "l %s", path);

    if (sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("client: sendto error");
        return -1;
      }

    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
        perror("client: recvfrom error");
        return -1;
      }
    printf("%d\n", buffer[0]);
    return buffer[0];
}

int tfsMount(char * sockPath) {
  server = sockPath;
  char client_name[1024];
  sprintf(client_name, "%s%d", "/tmp/client", getpid());

  printf("client name: %s\n", client_name);
  printf("server name: %s\n", server);

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
      perror("client: can't open socket");  
      return -1;
    }

    unlink(client_name); 

    clilen = setSockAddrUn (client_name, &client_addr);
  if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
      perror("client: bind error");
      return -1;
    }

  servlen = setSockAddrUn(server, &serv_addr);  
  if (servlen == 0) perror("wrong server address");
  return 0;
}

int tfsUnmount(char* argv[]) {
  close(sockfd);
  //unlink(argv[2]);
  return -1;
}
