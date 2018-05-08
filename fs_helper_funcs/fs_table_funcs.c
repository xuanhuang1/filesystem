#include "helper_funcs.h"
#include <assert.h>

extern fs_attr_t fs;


int extract_next_free_inode(){
	int ret_index = fs.freeiHead;
	fs.freeiHead = fs.inodes[fs.freeiHead].next_free_inode;
	if(ret_index == -1){
		printf("no inode left!\n");
		assert (ret_index != -1);
		return FAIL;
	}

	fs.inodes[ret_index].nlink = 1;
	fs.inodes[ret_index].size = 0;
	fs.inodes[ret_index].next_free_inode = EMPTY_ENTRY;

	printf("extract free inode:%d, set i head:%d\n", ret_index, fs.freeiHead);
	return ret_index;
}

int get_inode_in_open_table(int index){
	for (int i = 0; i < fs.table.length; ++i){
		if(fs.table.open_files[i].ind == index){
			return i;
		}
	}
	return FAIL;
}



int fd_is_valid(int fd){
	if((fs.table.open_files[fd].ind== EMPTY_ENTRY)||(fd > fs.table.length-1)){
		printf("file not in open table!\n");
		return FALSE;
	}
	return TRUE;
}

void init_new_file_inode(int i, int parent, int nlink, int size, int uid, int gid, int in_time){
	fs.inodes[i].parent = parent;
	fs.inodes[i].nlink = nlink;
	fs.inodes[i].size = size;
	fs.inodes[i].uid = uid;
	fs.inodes[i].gid = gid;
	fs.inodes[i].ctime = in_time;
	fs.inodes[i].mtime = in_time;
	fs.inodes[i].atime = in_time;
	fs.inodes[i].isdir = FALSE;
	for (int j = 0; j < N_DBLOCKS; ++j)
		fs.inodes[i].dblocks[j] = -1;
	
}

int add_one_entry_in_dir(int fd_for_cur_dir, dir_entry_t *dir_entry){

	printf("editting dir in table:%d\n", fd_for_cur_dir);
	int old_dir_mode = fs.table.open_files[fd_for_cur_dir].mode;
	fs.table.open_files[fd_for_cur_dir].mode = OPEN_APPEND;
	write_file_by_inode(&(fs.table.open_files[fd_for_cur_dir]), dir_entry, sizeof(dir_entry_t));
	fs.table.open_files[fd_for_cur_dir].mode = old_dir_mode;
	
	fs.inodes[fs.table.open_files[fd_for_cur_dir].ind].children_num ++;
	return SUCCESS;
}


int if_file_permission(inode_t i, int flag){
	int fileMode = i.permission;
	if(flag == OPEN_R){
		if(i.uid == fs.user) 			return (fileMode & S_IRUSR) && (fileMode & S_IREAD);
		if(i.gid == fs.u_gid[fs.user]) 	return (fileMode & S_IRGRP) && (fileMode & S_IREAD);
		else							return (fileMode & S_IROTH) && (fileMode & S_IREAD);
	}else if(flag == OPEN_W){
		if(i.uid == fs.user) 			return (fileMode & S_IRUSR) && (fileMode & S_IWRITE);
		if(i.gid == fs.u_gid[fs.user]) 	return (fileMode & S_IRGRP) && (fileMode & S_IWRITE);
		else				 			return (fileMode & S_IROTH) && (fileMode & S_IWRITE);
	}else{
		if(i.uid == fs.user) 			return (fileMode & S_IRUSR) && (fileMode & S_IEXEC);
		if(i.gid == fs.u_gid[fs.user]) 	return (fileMode & S_IRGRP) && (fileMode & S_IEXEC);
		else				 			return (fileMode & S_IROTH) && (fileMode & S_IEXEC);
	}
}
