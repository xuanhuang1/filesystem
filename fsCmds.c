#include "fsCmds.h"
#include "fs.h"
#include "fs_helper_funcs/helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define MAXFS 10
#define PATHMAX 255

typedef struct {
	fs_attr_t filesys;
	char wd[PATHMAX];
}mounted_fs_t;


mounted_fs_t m_fs_all[MAXFS];
int current_fs_index = -1;
int current_fs_total = 0;

extern fs_attr_t fs;

int shell_mount(char* filename){
	if(test_init_mount(filename) == FAIL){
		return FAIL;
	}
	char cwd[PATHMAX];
   	if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, "Mount fs at working dir: %s\n", cwd);
   	else{
       perror("getcwd() error");
   		return FAIL;
   	}

	current_fs_index = 0;
	m_fs_all[current_fs_index].filesys = fs;
	memcpy(m_fs_all[current_fs_index].wd, cwd, sizeof(cwd));
	current_fs_total ++;
	printf("disk:\"%s\" mounted, disk[%d]\n",filename, current_fs_index);
	return SUCCESS;
}


int shell_unmount(int index){
	if(index > current_fs_total || index < 0){
		printf("the fs[%d] does not exist\n", index);
		return FAIL;
	}
	free(m_fs_all[index].filesys.inodes);
	free(m_fs_all[index].filesys.table.open_files);
	m_fs_all[index] = m_fs_all[current_fs_total];
	current_fs_total--;
	return SUCCESS;
}

int shell_ls(){
	prt_dir_data(fs.shell_d);
	return SUCCESS;
}


int shell_chmod();

int shell_pwd(){
	int cur_dir = fs.shell_d;
	int num_of_dir = 0;
	while(cur_dir != fs.root){
		int parent = fs.inodes[cur_dir].parent;
		//char* name = search_name_in_dir(parent, cur_dir);
		//printf("/%s\n", name);
		//free(name);
		num_of_dir++;
		cur_dir = parent;
	}
	cur_dir = fs.shell_d;
	char** fullpath = (char**)malloc(sizeof(char*)*(num_of_dir+1));
	fullpath[num_of_dir] = NULL;

	for (int i = num_of_dir - 1; i > -1 ; --i){
		int parent = fs.inodes[cur_dir].parent;
		char* name = search_name_in_dir(parent, cur_dir);
		//printf("/%s\n", name);
		fullpath[i] = (char*)malloc(sizeof(char)*NAMELEN);
		memset(fullpath[i], 0, NAMELEN);
		memcpy(fullpath[i], name, NAMELEN);
		//printf("/%s\n", fullpath[i]);
		free(name);
		cur_dir = parent;
	}

	printf("/root");
	for (int i = 0; i < num_of_dir; ++i){
		printf("/%s", fullpath[i]);
		free(fullpath[i]);
	}
	free(fullpath);
	printf("\n");
	return SUCCESS;
}

int get_last_file_inode_in_path(char* pathname_in, char filename_buff[PATHMAX], char parentname_buff[PATHMAX]){
	char pathname[PATHMAX];
	memset(filename_buff, 0, PATHMAX);
	memset(pathname, 0, PATHMAX);
	memset(parentname_buff, 0, PATHMAX);

	memcpy(pathname, pathname_in, PATHMAX);
	pathname[PATHMAX-1] = '\0';
	filename_buff[PATHMAX-1] = '\0';
	parentname_buff[PATHMAX-1] = '\0';

	int cur_dir = -1;
	char* token = strtok(pathname, "/");

	// set cur dir to be the upper most dir
	// and token to be the name of the next file
	if(strcmp(token, ".")==0){
		cur_dir = fs.shell_d;
		token = strtok(NULL, "/");

	}else if(strcmp(token, "..")==0){
		cur_dir = fs.inodes[fs.shell_d].parent;
		token = strtok(NULL, "/");

	}else if(pathname[0] == '/'){
		cur_dir = fs.root;
	}else{
		printf("path name not correct!\n");
		return EMPTY_ENTRY;
	}
	char last_token[PATHMAX];
	memset(last_token, 0, PATHMAX);

	printf("first_dir:%d\n", cur_dir);
	while(token != NULL){
		printf("%s under %s", token, parentname_buff);
		// get the name
		memcpy(filename_buff, token, PATHMAX);
		memcpy(parentname_buff, last_token, PATHMAX);
		// get next
		token = strtok(NULL, "/");
		memcpy(last_token, filename_buff, PATHMAX);
		// if no next this is last file's parent
		if(!token){
			break;
		}
		// else get the next dir
		int inode = search_file_in_dir(cur_dir, filename_buff);
		if(inode == -1){
			printf("%s not found!\n", filename_buff);
			return EMPTY_ENTRY;
		}
		cur_dir = inode;
	}
	printf("\n");
	return cur_dir;
}

int shell_cd(char* pathname_in){
	char filename_buff[PATHMAX], parentname_buff[PATHMAX];
	int last_inode_parent = get_last_file_inode_in_path(pathname_in, filename_buff, parentname_buff);
	//printf("inode:%d filename:%s\n", last_inode, filename_buff);
	if(last_inode_parent == EMPTY_ENTRY){
		return FAIL;
	}
	int inode = search_file_in_dir(last_inode_parent, filename_buff);
	if(inode == EMPTY_ENTRY){
		return FAIL;
	}
	fs.shell_d = inode;;
	return SUCCESS;
}

int shell_mkdir(char* pathname){
	char filename_buff[PATHMAX], parentname_buff[PATHMAX];
	int last_inode_parent = get_last_file_inode_in_path(pathname, filename_buff, parentname_buff);
	//printf("inode:%d filename:%s\n", last_inode, filename_buff);
	if(last_inode_parent == EMPTY_ENTRY){
		return FAIL;
	}
	// if not open yet open parent dir
	int fd = -1;
	if(get_inode_in_open_table(last_inode_parent) == EMPTY_ENTRY){
		fd = f_opendir(fs.inodes[last_inode_parent].parent, parentname_buff);
	}
	if(f_mkdir(last_inode_parent, filename_buff) == FAIL)
		return FAIL;
	if(fd > 0) f_closedir(fd);
	return SUCCESS;
}
int shell_rmdir(char* pathname){
	char filename_buff[PATHMAX], parentname_buff[PATHMAX];
	int last_inode_parent = get_last_file_inode_in_path(pathname, filename_buff, parentname_buff);
	//printf("inode:%d filename:%s\n", last_inode, filename_buff);
	if(last_inode_parent == EMPTY_ENTRY){
		return FAIL;
	}
	if(f_rmdir(last_inode_parent, filename_buff) == FAIL)
		return FAIL;
	return SUCCESS;

}
int shell_cat(char* pathname);
int shell_more(char* pathname);
int shell_rm(char* pathname);