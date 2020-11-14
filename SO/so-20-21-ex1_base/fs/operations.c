#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 */
void split_parent_child_from_path(char * path, char ** parent, char ** child) {

	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len-1] == '/') {
		path[len-1] = '\0';
	}

	for (int i=0; i < len; ++i) {
		if (path[i] == '/' && path[i+1] != '\0') {
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0) { // root directory
		*parent = "";
		*child = path;
		return;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;

}


void unlock_all(int *to_unlock);


/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs() {
	inode_table_init();
	
	/* create root inode */
	int root = inode_create(T_DIRECTORY);
	
	if (root != FS_ROOT) {
		printf("failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}


/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs() {
	inode_table_destroy();
}


/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */

int is_dir_empty(DirEntry *dirEntries) {
	if (dirEntries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (dirEntries[i].inumber != FREE_INODE) {
			return FAIL;
		}
	}
	return SUCCESS;
}


/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0) {
            return entries[i].inumber;
        }
    }
	return FAIL;
}


/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;
	//int e = 0;
	int to_unlock[100];

	int iter[1] = {0};
	
	for (int a = 0; a < 100 ; a++){ 
        to_unlock[a] = -33; 
    }

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = aux_lookup(parent_name, to_unlock, WRITELOCK, iter);

	if (parent_inumber == FAIL) {
		printf("failed to create %s, invalid parent dir %s\n",
		        name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to create %s, parent %s is not a dir\n",
		        name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL) {
		printf("failed to create %s, already exists in dir %s\n",
		       child_name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);
	lock(child_inumber, WRITELOCK);
	if (child_inumber == FAIL) {
		printf("failed to create %s in  %s, couldn't allocate inode\n",
		        child_name, parent_name);
		unlock(child_inumber);
		unlock_all(to_unlock);
		return FAIL;
	}

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL) {
		printf("could not add entry %s in dir %s\n",
		       child_name, parent_name);
		unlock(child_inumber);
		unlock_all(to_unlock);
		return FAIL;
	}
	unlock(child_inumber);
	unlock_all(to_unlock);
	return SUCCESS;
}


/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete(char *name){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;
	//int e = 0;
	int to_unlock[100];

	int iter[1] = {0};

	for (int a = 0; a < 100 ; a++){ 
        to_unlock[a] = -33; 
    }


	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = aux_lookup(parent_name, to_unlock, WRITELOCK, iter);

	if (parent_inumber == FAIL) {
		printf("failed to delete %s, invalid parent dir %s\n",
		        child_name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to delete %s, parent %s is not a dir\n",
		        child_name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}
	
	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);
	
	

	if (child_inumber == FAIL) {
		printf("could not delete %s, does not exist in dir %s\n",
		       name, parent_name);
		unlock(child_inumber);
		unlock_all(to_unlock);
		return FAIL;
	}
	lock(child_inumber, WRITELOCK);
	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL) {
		printf("could not delete %s: is a directory and not empty\n",
		       name);
		unlock(child_inumber);
		unlock_all(to_unlock);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL) {
		printf("failed to delete %s from dir %s\n",
		       child_name, parent_name);
		unlock(child_inumber);
		unlock_all(to_unlock);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL) {
		printf("could not delete inode number %d from dir %s\n",
		       child_inumber, parent_name);
		unlock(child_inumber);
		unlock_all(to_unlock);
		return FAIL;
	}
	unlock(child_inumber);
	unlock_all(to_unlock);
	return SUCCESS;
}


/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup(char *name) {
	int to_unlock[100];
	int iter[1] = {0};

	for (int a = 0; a < 100 ; a++){ 
        to_unlock[a] = -33; 
    }

	int current_inumber = aux_lookup(name, to_unlock, READLOCK, iter);

	unlock_all(to_unlock);
	return current_inumber;
}


int move(char* origin, char* destination) {

	int parent_dest_inumber;
	int child_dest_inumber;
	int origin_inumber;
	int origin_parent_inumber;
	char origin_copy[MAX_FILE_NAME];
	char origin_copy2[MAX_FILE_NAME];
	char destination_copy[MAX_FILE_NAME];
	char destination_copy2[MAX_FILE_NAME];

	char *origin_p_name, *origin_c_name;
	char *parent_name, *child_name;

	//int w =0;
 	int iter[1] = {0};

	
	
	int to_unlock[100];
	
	for (int a = 0; a < 100 ; a++){ 
        to_unlock[a] = -33; 
    }


	strcpy(origin_copy, origin);
	strcpy(origin_copy2, origin);
	strcpy(destination_copy, destination);
	strcpy(destination_copy2, destination);

	split_parent_child_from_path(origin_copy, &origin_p_name, &origin_c_name);
	split_parent_child_from_path(destination_copy, &parent_name, &child_name);

	if (strcmp(origin_p_name, parent_name) < 0) {
		origin_parent_inumber = aux_lookup(origin_p_name, to_unlock, WRITELOCK, iter);		
		origin_inumber = aux_lookup(origin_copy2, to_unlock, WRITELOCK, iter);

		parent_dest_inumber = aux_lookup(parent_name, to_unlock, WRITELOCK, iter);		
		child_dest_inumber = aux_lookup(destination_copy2, to_unlock, WRITELOCK,iter);
		
	}
	else
	{
		parent_dest_inumber = aux_lookup(parent_name, to_unlock, WRITELOCK, iter);
		child_dest_inumber = aux_lookup(destination_copy2, to_unlock, WRITELOCK, iter);
		
		origin_parent_inumber = aux_lookup(origin_p_name, to_unlock, WRITELOCK, iter);
		origin_inumber = aux_lookup(origin_copy2, to_unlock, WRITELOCK, iter);
	}

	
	
	if (origin_inumber == FAIL) {
		printf("failed to move %s, invalid file/dir %s\n",
		        origin, origin_p_name);
		unlock_all(to_unlock);
		return FAIL;
	}
	if (child_dest_inumber != FAIL) {
		printf("failed to move %s, invalid destination dir/file already exists %s\n",
		        origin, destination);
		unlock_all(to_unlock);
		return FAIL;
	}
	if (parent_dest_inumber == FAIL) {
		printf("failed to move %s, invalid destination parent dir %s\n",
				child_name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}
	if (dir_add_entry(parent_dest_inumber, origin_inumber, child_name) == FAIL) {
		printf("could not move entry %s in dir %s\n",
			child_name, parent_name);
		unlock_all(to_unlock);
		return FAIL;
	}

	if (dir_reset_entry(origin_parent_inumber, origin_inumber) == FAIL) {
		printf("failed to move %s from dir %s\n",
		       origin_c_name, origin_p_name);
		unlock_all(to_unlock);
		return FAIL;
	}
	
	unlock_all(to_unlock);
	return SUCCESS;
}




int check_lock(int inumber, int *to_unlock) {
	int i = 0;
	while (to_unlock[i] != -33) {
		if(inumber == to_unlock[i]) {
			return SUCCESS;
		}
		i++;
	}
	return FAIL;
}

int aux_lookup(char *name, 	int *to_unlock, int mode, int *i) {
	char full_path[MAX_FILE_NAME];
	char *saveptr;
	char delim[] = "/";
	//int i = 0;
	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	char *path = strtok_r(full_path, delim, &saveptr);

	/* get root inode data */
	if(!path && mode == WRITELOCK){
		if(check_lock(current_inumber, to_unlock) == FAIL) {
			lock(current_inumber, WRITELOCK);//locks root node
			to_unlock[i[0]] = current_inumber;
			i[0]++;
		}
	}
	else{
		if(check_lock(current_inumber, to_unlock) == FAIL) {
			lock(current_inumber, READLOCK);//locks root node
			to_unlock[i[0]] = current_inumber;
			i[0]++;
		}
	}

	
	inode_get(current_inumber, &nType, &data);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		path = strtok_r(NULL, delim, &saveptr); 
		if(!path && mode == WRITELOCK){
			if(check_lock(current_inumber, to_unlock) == FAIL) {
				lock(current_inumber, WRITELOCK);//locks every node till last one in the path for read
				to_unlock[i[0]] = current_inumber;
				i[0]++;
			}
		}
		else{
			if(check_lock(current_inumber, to_unlock) == FAIL) {
				lock(current_inumber, READLOCK);
				to_unlock[i[0]] = current_inumber;
				i[0]++;
			}
		}

		inode_get(current_inumber, &nType, &data);
	}
	//unlock(FS_ROOT);
	return current_inumber;
}




void unlock_all(int *to_unlock){
	int i = 0;
	while (to_unlock[i] != -33){
		unlock(to_unlock[i]);
		i++;
	}
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp){
	inode_print_tree(fp, FS_ROOT, "");
}
