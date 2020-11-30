#ifndef API_H
#define API_H
#define MAX_INPUT_SIZE 100
#include "tecnicofs-api-constants.h"
#include <sys/socket.h>

int tfsCreate(char *path, char nodeType);
int tfsDelete(char *path);
int tfsLookup(char *path);
int tfsMove(char *from, char *to);
int tfsPrint(char *path);
int tfsMount(char* serverName);
int tfsUnmount(char* argv[]);

#endif /* CLIENT_H */
