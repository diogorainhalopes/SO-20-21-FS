#ifndef FS_H
#define FS_H
#include "state.h"

#define LOOKWRITE 1
#define LOOKONLY 2

void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete(char *name);
int lookup(char *name);
int aux_lookup(char *name, 	int to_unlock[], int mode);
void print_tecnicofs_tree(FILE *fp);

#endif /* FS_H */
