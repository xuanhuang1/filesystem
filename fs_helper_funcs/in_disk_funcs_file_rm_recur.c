#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;


// find the inode by filename at given directory current_d, if not found return -1 and set errno
// if nlink==0 or permission doesn’t allow read return -1 and set errno
// if not a directory remove the file by calling f_remove(current_d, current_d.name);
// else:    while current_d.children_num != 0
//        get the inode for the child file i = inodes[current_d.nextfile]
//        if isdir==TRUE open the child by fd_child = f_opendir(i, i.name)
//            recursively call f_rmdir(fs.table[fd_child].ind, 
//        if isdir== FALSE open the child by fd_child = f_open(i, i.name)
//            call f_remove(fs.table[fd_child].ind, fs.table[fd_child].ind.name)
//    when all the child files/directories are removed 
//    call f_remove(current_d, current_d.name) to remove the empty directory itself
int f_rmdir_recur(int dir_index, char* filename){
	printf("Removing \"%s\" under:%d\n",filename, dir_index);

	int fd = f_opendir(dir_index, filename);
	int this_dir_inode_num = fs.table.open_files[fd].ind;
	if(fs.inodes[this_dir_inode_num].children_num == 0){
		assert(fs.inodes[this_dir_inode_num].size == 0);
		printf("remove empty dir %d\n", this_dir_inode_num);

		delete_file_by_inode(this_dir_inode_num);
		f_closedir(fd);
		return SUCCESS;
	}
	printf("this dir: %d\n", fs.table.open_files[fd].ind);

	f_rewind(fd);
	for(int i=0; i<fs.inodes[this_dir_inode_num].children_num; i++){
		dir_entry_t dir_entry;

		f_readdir(fd, &dir_entry);
		printf("\tthe next file to remove %d \"%s\" isdir?%d, under dir:%d\n", 
			dir_entry.ind, dir_entry.name,
			fs.inodes[dir_entry.ind].isdir, this_dir_inode_num);
		if(fs.inodes[dir_entry.ind].isdir == FALSE){
			printf("\trm file %d in rm dir %d\n", dir_entry.ind, this_dir_inode_num);

			//trunc_file(&fs.inodes[dir_entry.ind]);
			//fs.inodes[dir_entry.ind].nlink = 0;
			delete_file_by_inode(dir_entry.ind);
		}
		else f_rmdir_recur(this_dir_inode_num, dir_entry.name);
	}
	// now the dirctory is empty, remove the inode
	//fs.inodes[this_dir_inode_num].size = 0;
	fs.inodes[this_dir_inode_num].children_num = 0;
	delete_file_by_inode(this_dir_inode_num);

	f_closedir(fd);
	return SUCCESS;
}