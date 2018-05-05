#include "fs.h"
#include "fs_helper_funcs/helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

fs_attr_t fs;

int test_init_mount(){
	// read the size of file
	// init write buffer
	FILE *ptr_ipt = fopen("disk","r");
	char *infile_buff = NULL;
	int sz = get_file_size(ptr_ipt, &infile_buff);

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
	load_fs(*out_spb, SUPERUSER, inodes, inode_count, "disk", db_num);

	// add root to the open file table
	f_entry_t root_file_entry;
	root_file_entry.mode = OPEN_RW;
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

	printf("\n\nOPENING %s under dir with inode:%d\n", filename, dir_index);

	int inode_index = search_file_in_dir(dir_index, filename);
	if(get_inode_in_open_table(inode_index) != FAIL){
		printf("FAILURE!!!inode:%d already in open table[%d]\n", 
			inode_index, get_inode_in_open_table(inode_index));
		return FAIL;
	}
	f_entry_t the_file_entry;
	the_file_entry.mode = flags;
	the_file_entry.offset = 0;

	if(flags == OPEN_R){ // add persmission check!!
		if(inode_index == -1){
			printf("FAILURE!!! %s not found in dir[%d], CANNOT READ\n", filename, dir_index);
			return FAIL;
		}else	the_file_entry.ind = inode_index;

	}else if((flags == OPEN_W) || (flags == OPEN_RW) || (flags == OPEN_APPEND)){

		if(inode_index == -1){
			printf("%s not found in dir[%d], creating\n", filename, dir_index);
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
			int fd_for_cur_dir = get_inode_in_open_table(dir_index);
			printf("editting dir in table:%d\n", fd_for_cur_dir);
			write_file_by_inode(&(fs.table.open_files[fd_for_cur_dir]), &dir_entry, sizeof(dir_entry_t));


			fs.inodes[dir_index].children_num ++;

		}else 	the_file_entry.ind = inode_index;
	}

	fs.table.open_files[fs.table.length] = the_file_entry;
	fs.table.length++;
	printf("fill table[%d] with inode %d, flag:%d\n",
		fs.table.length-1, the_file_entry.ind, the_file_entry.mode);
	printf("\nEND ADDING OPEN TABLE\n\n");
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

	f_entry_t *the_file_entry = &fs.table.open_files[fd];
	int bytes_written = write_file_by_inode(the_file_entry, buf, len);
	return bytes_written;
}

int f_read(int fd, void *buf, int len){
	if(fd_is_valid(fd) == FALSE)
		return FAIL;

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
int fstat(int fd, struct stat *buf);

// find the inode by filename at given directory current_d
// if it is a directory with children or nlink==0 return -1 and set errno
// else set nlink = 0, remove the inode from current_d’s children list 
// and swap the last child inode there
// decrement children_num for the current_d’s inode
// return 1 when success, return -1 when fails and set errno
// will NOT close the file if it is in the open file table
int f_remove(int dir_index, char filename);


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
int f_readdir(int fd);

// same as f_close(fd)
int f_closedir(int fd);

// get a file descriptor fd = f_open(current_d, filename, RW); 
// set the inode’s isdir to TRUE
// f_close(fd)
int f_mkdir(int dir_index, char* filename){
	int fd_dir = f_open(dir_index, filename, OPEN_RW);
	if(fd_dir == FAIL) return FAIL;
	inode_t *inode = &(fs.inodes[fs.table.open_files[fd_dir].ind]);
	printf("Making dir for inode[%d], at fd:%d\n", fs.table.open_files[fd_dir].ind, fd_dir);
	inode->isdir = TRUE;
	f_close(fd_dir);
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
int f_rmdir(dir_entry_t current_d, char filename);

// look for DISK and call fs_init()
// if not found call format() then fs_init()
// malloc a fs_attr_t for the file system
// return ptr to fs_attr_t when success, return -1 when fs_init() or format() fails
fs_attr_t* f_mount(char *dir, int flags);

// if any file other than root and user directories left and flag not MNT_FORCE
// return NULL and set errno
// else return fs
fs_attr_t* f_unmount(char *dir, int flags);


char* fill_buffer_from_FILE(char* filename, int* size){
	FILE *f = fopen(filename,"r+");
	char *infile_buff = NULL;
	*size = get_file_size(f, &infile_buff);
	read_with_fread(infile_buff, sizeof(char), *size, f);
	printf("file:%s origianl sz %d\n", filename, *size);
	fclose(f);
	return infile_buff;
}

void test_create_and_append(int fd){
	//dir_entry_t *dir = (dir_entry_t*)get_one_data_block(0, 0, sizeof(dir_entry_t));
	//printf("dir_entry %s %d\n", dir->name, dir->ind);
	int sz = 0;
	char* infile_buff = fill_buffer_from_FILE("test_data_file/data1.txt", &sz);
	int bytes_written = f_write(fd, infile_buff, sz);
	free(infile_buff);

	printf("\nEnd of f_write. bytes_written:%d\n", bytes_written);

	printf("\n\n\n\n END OF CREATE \n\n\n\n");


	int sz2 = 0;
	char* infile_buff2 = fill_buffer_from_FILE("test_data_file/append2.txt", &sz2);
	int bytes_written2 = f_write(fd, infile_buff2, sz2);
	free(infile_buff2);

	printf("\nEnd of f_write. bytes_written2:%d\n", bytes_written2);

	printf("\n\n\n\n END OF APPEND \n\n\n\n");

	//prt_file_data(fs.table.open_files[fd].ind);


}

void test_read_twice(int fd){
	char* readed_data = (char*)malloc(514);
	char* readed_data2 = (char*)malloc(5);

	readed_data[513] = '\0';
	readed_data2[4] = '\0';

	f_rewind(1);
	int bytes_read = f_read(1, readed_data, 513);
	printf("\nEnd of f_read. bytes_read:%d\n", bytes_read);

	int bytes_read2 = f_read(1, readed_data2, 4);
	printf("\nEnd of f_read. bytes_read2:%d\n", bytes_read2);

	printf("\n\n\n\n END OF READ \n\n\n\n");
	printf("read buffer:\n");
	printf("%s + %s\n", readed_data, readed_data2);
	free(readed_data);
	free(readed_data2);

}

void test_fopen_invalid(){
	f_mkdir(0, "user");

	int fd_dir = f_opendir(0, "user");
	int fd = f_open(1, "file1", OPEN_RW);
	prt_fs();

}

int main(){
	// one spb, one blk (4) inodes, 2 data blocks (one for root)
	format_disk();
	test_init_mount();
	prt_fs();
	prt_table();

	printf("\n\n\n\n END OF INIT \n\n\n\n");
	//printf("d entry size:%d, num:%d\n", sizeof(dir_entry_t), fs.spb.size/sizeof(dir_entry_t));




	//prt_data_region();


	//int next_inode = extract_next_free_inode();
	//create_file_at_inode(next_inode, "test_data_file/data1.txt");
	int fd = f_open(0, "file1",OPEN_APPEND);
	test_create_and_append(fd);
	test_read_twice(fd);
	
	f_close(fd);
	
	/*
	int fd2 = f_open(0, "file1",OPEN_R);
	test_read_twice(fd2);
	f_close(fd2);*/
	//f_mkdir(0, "dir1");
	//test_fopen_invalid();	
	prt_table();

	free(fs.inodes);
	free(fs.table.open_files);
} 

//// HELPER FUNCS ///// 

