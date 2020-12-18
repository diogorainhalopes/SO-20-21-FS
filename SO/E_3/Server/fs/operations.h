#ifndef FS_H
#define FS_H
#include "state.h"

void unlock_all(int *to_unlock);
void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete(char *name);
int lookup(char *name);
int move(char* orig, char* dest);
int check_lock(int inumber, int *to_unlock);
int aux_lookup(char *name, 	int to_unlock[], int mode, int *i);
void print_tecnicofs_tree(FILE *fp);

#endif /* FS_H */
