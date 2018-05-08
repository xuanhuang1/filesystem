#include "fs.h"
#include "fs_helper_funcs/helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

fs_attr_t fs;

int test_init_mount(char* filename){
	// read the size of file
	// init write buffer
	FILE *ptr_ipt = fopen(filename,"r");
	char *infile_buff = NULL;
	int sz = get_file_size_with_fseek(ptr_ipt, &infile_buff);

	// read file into char buffer
	//printf("\nopen disk!\n");
	read_with_fread(infile_buff, sizeof(char), sz, ptr_ipt);
	fclose(ptr_ipt);

	// read spb and store into local varibles
	spb_t *out_spb = (spb_t*)infile_buff;
	inode_t *inodes = (inode_t*)(infile_buff + 512 + out_spb->inode_offset);
	inode_t *out_root = inodes;
	int blocksize = out_spb->size;
	int db_num = (sz - blocksize*(1+out_spb->data_offset))/ blocksize;
	int inode_count = get_inode_count(*out_spb);
	
	// print local varibles
	//prt_spb(*out_spb);
	//prt_root(*out_root);
	//for (int i = 0; i < inode_count-1; ++i)
	//	prt_inode(inodes[i+1]);
	//printf("sz:%d db_num %d\n", sz,db_num);


	// load fs gloabls from the local varibles
	load_fs(*out_spb, SUPERUSER, inodes, inode_count, filename, db_num);

	// add root to the open file table
	f_entry_t root_file_entry;
	root_file_entry.mode = OPEN_R;
	root_file_entry.offset = 0;
	root_file_entry.ind = 0;

	fs.table.open_files = (f_entry_t*)malloc(inode_count*sizeof(f_entry_t));
	//for (int i = 0; i < inode_count; ++i)
	//	fs.table.open_files[i].ind = EMPTY_ENTRY;
	fs.table.open_files[0] = root_file_entry;
	fs.table.length = 1;

	free(infile_buff);


	return SUCCESS;

}

/*
 find the inode by filename at given directory current_d
 check permission, if doesn’t match with flag return -1 and set errno
 if reading: if not found return error
 if writing/appending:
     if not found,  find the first empty entry in fs.table and create a f_entry_t obj
        assign a inode in fs.freeiHead
        set the new inode’s parent to current_d
        add the new inode’s inode number into current_d’s data region
     if no fs.freeiHead return error, else set new fs.freeiHead
 return the index of new file in fs.table when success
 */
int f_open(int dir_index, char *filename, int flags){
	assert (dir_index > -1);
	inode_t current_d = fs.inodes[dir_index];
	assert (current_d.nlink > 0);
	assert (current_d.isdir == 1); 

	printf("\n\nOPENING \"%s\" under dir with inode:%d\n", filename, dir_index);

	int inode_index = search_file_in_dir(dir_index, filename);
	if(get_inode_in_open_table(inode_index) != FAIL){
		printf("DUPLICATE!!!inode:%d already in open table[%d]\n", 
			inode_index, get_inode_in_open_table(inode_index));
		return inode_index;
	}
	f_entry_t the_file_entry;
	the_file_entry.mode = flags;
	the_file_entry.offset = 0;

	if(flags == OPEN_R){ // add persmission check!!
		if(inode_index == -1){
			printf("FAILURE!!! \"%s\" not found in dir[%d], CANNOT READ\n", filename, dir_index);
			return FAIL;
		}else	the_file_entry.ind = inode_index;

	}else if((flags == OPEN_W) || (flags == OPEN_RW) || (flags == OPEN_APPEND)){

		if(inode_index == -1){
			printf("\"%s\" not found in dir[%d], creating\n", filename, dir_index);
			int fd_for_cur_dir = get_inode_in_open_table(dir_index);
			if(fd_for_cur_dir == FAIL){
				printf("parent directory %d not open!\n", dir_index);
				return FAIL;
			}

			the_file_entry.ind = extract_next_free_inode();	
			init_new_file_inode(the_file_entry.ind, dir_index, 1, 0, fs.user, fs.u_gid[fs.user], 1234);

			// write to incoming dir

			dir_entry_t dir_entry;
			dir_entry.ind = the_file_entry.ind;
			memcpy(dir_entry.name, filename, NAMELEN);

			//buffer[15] = '\0';
			//printf("entry written buffer:%s\n", buffer);
			// open dir and f_write append
			// hard coded here
			add_one_entry_in_dir(fd_for_cur_dir, &dir_entry);


		}else 	the_file_entry.ind = inode_index;
	}

	fs.table.open_files[fs.table.length] = the_file_entry;
	fs.table.length++;
	printf("fill table[%d] with inode %d, flag:%d\n",
		fs.table.length-1, the_file_entry.ind, the_file_entry.mode);
	printf("\nEnd adding open table\n\n");
	return fs.table.length-1;

	
	return FAIL;
}


// if fd < fs.table.length get the file entry fs.table[fd], get the inode i = fs.table[fd].ind
//    if nlink!=0, set the file’s offset to the end if O_APPEND
//    store count number of bytes from data to i, if current blocks not enough set indirect ptr
//     return the number of bytes written, if error return -1 and set errno
//    if nlink==0 or if fd >= fs.table.length or permission not match return -1 and set errno
int f_write(int fd, void *buf, int len){
	if(fd_is_valid(fd) == FALSE)
		return FAIL;
	if(len == 0) return 0;
	f_entry_t *the_file_entry = &fs.table.open_files[fd];
	int bytes_written = write_file_by_inode(the_file_entry, buf, len);
	return bytes_written;
}

int f_read(int fd, void *buf, int len){
	if(fd_is_valid(fd) == FALSE)
		return FAIL;
	if(len == 0) return 0;

	f_entry_t *the_file_entry = &fs.table.open_files[fd];
	int bytes_read = read_file_by_inode(the_file_entry, buf, len);
	return bytes_read;
}


// if fs.table[fd]  == -1 or fd >= fs.table.length return error
// set fs.table[fd]->offset whence at offset
// If whence is set to SEEK_BEGIN, SEEK_CURR, or SEEK_ENDFILE
// the offset is relative to the start of the file,the current position indicator, or end-of-file, respectively.
int f_seek(int fd, int offset, int whence){
	if(fd_is_valid(fd) == FALSE)
		return FAIL;
	
	if(whence == SEEK_BEGIN){
		if(fs.inodes[fs.table.open_files[fd].ind].size < offset){
			printf("SETTING SEEK BEYOND SIZE: BEGIN! %d\n", offset);
			return FAIL;
		}
		fs.table.open_files[fd].offset = offset;
		return SUCCESS;
	}
	if(whence == SEEK_CURR){
		if(fs.inodes[fs.table.open_files[fd].ind].size < offset+fs.table.open_files[fd].offset){
			printf("SETTING SEEK BEYOND SIZE: CURR! %d\n", offset);
			return FAIL;
		}
		fs.table.open_files[fd].offset += offset;
		return SUCCESS;
	}
	if(whence == SEEK_ENDFILE){
		fs.table.open_files[fd].offset = fs.inodes[fs.table.open_files[fd].ind].size;
		return SUCCESS;
	}else{
		printf("Wrong flag in seek\n");
		return FAIL;
	}
}

// set fs.table[fd]->offset to 0
int f_rewind(int fd){
	if(fd_is_valid(fd) == FALSE)
		return FAIL;
	fs.table.open_files[fd].offset = 0;
	return SUCCESS;
}

// if fs.table[fd]  == -1 or fd >= fs.table.length return error
// else set fs.table[fd] to -1, swap the last entry in fs.table to fs.table[fd]
// decrement fs.table.length
// return 0 on success, return -1 and set errno if fail
int f_close(int fd){
	if(fd_is_valid(fd) == FALSE)
		return FAIL;

	if(fs.table.length-1 == 0) printf("CLOSING LAST ENTRY!!!\n");

	f_entry_t last_entry = fs.table.open_files[fs.table.length-1];
	fs.table.open_files[fd] = last_entry;
	fs.table.open_files[fs.table.length-1].ind = EMPTY_ENTRY;
	printf("Close table[%d], swap[%d] there, now table[%d].ind=%d\n",
	 fd, fs.table.length-1, fs.table.length-1, fs.table.open_files[fs.table.length-1].ind);
	
	fs.table.length--;
	return SUCCESS;
}

// read fs.table[fd]->ind ‘s  attribute to a struct stat
int f_stat(int fd, fs_stat_t *buf){
	inode_t inode = fs.inodes[fs.table.open_files[fd].ind];
	buf->mode = fs.table.open_files[fd].mode;
	buf->nlink = inode.nlink;
	buf->size = inode.size;
	buf->uid = inode.uid;
	buf->gid = inode.gid;
	buf->ctime = inode.ctime;
	buf->mtime = inode.mtime;
	buf->atime = inode.atime;
	return SUCCESS;
}

// find the inode by filename at given directory current_d
// if it is a directory with children or nlink==0 return -1 and set errno
// else set nlink = 0, remove the inode from current_d’s children list 
// and swap the last child inode there
// decrement children_num for the current_d’s inode
// return 1 when success, return -1 when fails and set errno
// will NOT close the file if it is in the open file table
int f_remove(int dir_index, char* filename){
	//printf("\n removing \"%s\" under dir with inode:%d\n", filename, dir_index);

	// clear entry in parent dir
	dir_entry_t last_dir_entry;
	remove_last_entry_in_dir(dir_index, &last_dir_entry);
	int inode_of_file = find_replace_dir_entry(dir_index, filename, &last_dir_entry);
	if(inode_of_file != FAIL){
		delete_file_by_inode(inode_of_file);
		fs.inodes[dir_index].children_num--;
		fs.inodes[dir_index].size -= sizeof(dir_entry_t);
	}else{
		printf("the entry to remove: \"%s\" under inode[%d] does not exist\n", filename, dir_index);
	}
	//printf("last dir entry removed: %d %s\n\n", last_dir_entry.ind, last_dir_entry.name);
	return SUCCESS;
}


// find the inode by filename at given directory current_d, if not found return -1 and set errno
// if not a directory or nlink==0 or permission doesn’t allow read return -1 and set errno
// else find the first empty entry in fs.table and create a f_entry_t object
// return the index of the newly created f_entry_t object in fs_table
int f_opendir(int dir_index, char* filename){
	int fd_dir = f_open(dir_index, filename, OPEN_R);
	assert (fs.inodes[fs.table.open_files[fd_dir].ind].isdir == TRUE);
	return fd_dir;
}


// if fd < fs.table.length get the file entry fs.table[fd], get the inode i = fs.table[fd].ind
// if not a directory or nlink==0 or permission doesn’t match flag return -1 and set errno
// set new i.nextfile to the next children file of current_d
// return the first inode whose isdir == TRUE after the old i.nextfile
int f_readdir(int fd, dir_entry_t *dir_read){
	if(fs.inodes[fs.table.open_files[fd].ind].isdir == FALSE){
		printf("!!Trying to read a non-directory %d with f_readdir!!\n", fs.table.open_files[fd].ind);
		assert (fs.inodes[fs.table.open_files[fd].ind].isdir == TRUE);
		return FAIL;
	}//printf("!!Trying to access a directory larger than cur size!!%d %d\n",
	//		fs.table.open_files[fd].offset, fs.inodes[fs.table.open_files[fd].ind].size);
		
	if(!(fs.table.open_files[fd].offset < fs.inodes[fs.table.open_files[fd].ind].size)){
		return FAIL;

	}
	if(f_read(fd, dir_read, sizeof(dir_entry_t)))
		return SUCCESS;
	return FAIL;
}

// same as f_close(fd)
int f_closedir(int fd){
	if(fs.inodes[fs.table.open_files[fd].ind].isdir == FALSE){
		printf("!!Trying to close a non-directory %d with f_closedir!!\n",fs.table.open_files[fd].ind );
		assert (fs.inodes[fs.table.open_files[fd].ind].isdir == TRUE);
		return FAIL;
	}
	f_close(fd);
	return SUCCESS;
}

// get a file descriptor fd = f_open(current_d, filename, RW); 
// set the inode’s isdir to TRUE
// f_close(fd)
int f_mkdir(int dir_index, char* filename){
	int fd_dir = f_open(dir_index, filename, OPEN_RW);
	if(fd_dir == FAIL) return FAIL;
	inode_t *inode = &(fs.inodes[fs.table.open_files[fd_dir].ind]);
	printf("Making dir for inode[%d], at temp fd:%d (close later)\n", fs.table.open_files[fd_dir].ind, fd_dir);
	inode->isdir = TRUE;
	f_closedir(fd_dir);
	return SUCCESS;
}

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
int f_rmdir(int dir_index, char* filename){
	f_rmdir_recur(dir_index, filename);

	// clear entry in parent dir
	dir_entry_t last_dir_entry;
	remove_last_entry_in_dir(dir_index, &last_dir_entry);
	int inode_of_file = find_replace_dir_entry(dir_index, filename, &last_dir_entry);
	if(inode_of_file != FAIL){
		fs.inodes[dir_index].children_num--;
		fs.inodes[dir_index].size -= sizeof(dir_entry_t);
	}else{
		printf("the entry to remove: \"%s\" under inode[%d] does not exist\n", filename, dir_index);
	}
	return SUCCESS;
}

// look for DISK and call fs_init()
// if not found call format() then fs_init()
// malloc a fs_attr_t for the file system
// return ptr to fs_attr_t when success, return -1 when fs_init() or format() fails
fs_attr_t* f_mount(char *dir, int flags);

// if any file other than root and user directories left and flag not MNT_FORCE
// return NULL and set errno
// else return fs
fs_attr_t* f_unmount(char *dir, int flags);


