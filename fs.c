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
	root_file_entry.mode = -3;
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
int f_open(int dir_index, const char *filename, int flags){
	assert (dir_index > -1);
	inode_t current_d = fs.inodes[dir_index];
	assert (current_d.nlink > 0);
	assert (current_d.isdir == 1);
	if(flags == R){ // add persmission check!!
		//buffer 
	}
	return SUCCESS;
}


// if fd < fs.table.length get the file entry fs.table[fd], get the inode i = fs.table[fd].ind
//    if nlink!=0, set the file’s offset to the end if O_APPEND
//    store count number of bytes from data to i, if current blocks not enough set indirect ptr
//     return the number of bytes written, if error return -1 and set errno
//    if nlink==0 or if fd >= fs.table.length or permission not match return -1 and set errno
int f_write(int fd, void *buf, int len){
	f_entry_t *the_file_entry = &fs.table.open_files[fd];
	write_file_by_inode(the_file_entry, buf, len);
	return 0;
}

int f_read(int fd, void *buf, int len){
	f_entry_t *the_file_entry = &fs.table.open_files[fd];
	read_file_by_inode(the_file_entry, buf, len);
	return 0;
}


int f_seek(int fd, int offset, int whence){
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

void f_rewind(int fd){
	fs.table.open_files[fd].offset = 0;
}


void create_file_at_inode(int file_i_index, char* filename){
	f_entry_t the_file_entry;
	the_file_entry.mode = OPEN_APPEND;
	the_file_entry.offset = 0;
	the_file_entry.ind = file_i_index;
	fs.table.open_files[fs.table.length] = the_file_entry;
	
	fs.inodes[file_i_index].nlink = 1;
	fs.inodes[file_i_index].parent = 0;
	fs.inodes[file_i_index].size = 0;


	fs.table.length++;


	FILE *f = fopen(filename,"r+");
	char *infile_buff = NULL;
	int sz = get_file_size(f, &infile_buff);
	read_with_fread(infile_buff, sizeof(char), sz, f);
	printf("file:%s origianl sz %d\n", filename, sz);
	fclose(f);
	f_write(fs.table.length-1, infile_buff, sz);
	free(infile_buff);


	prt_file_data(file_i_index);
	prt_data_region();
}

void append_file_at_inode(int file_i_index, char* filename){
	FILE *f = fopen(filename,"r+");
	char *infile_buff = NULL;
	int sz = get_file_size(f, &infile_buff);
	read_with_fread(infile_buff, sizeof(char), sz, f);
	printf("file:%s origianl sz %d\n", filename, sz);
	fclose(f);

	f_write(fs.table.length-1, infile_buff, sz);
	free(infile_buff);


	prt_file_data(file_i_index);
}


int main(){
	// one spb, one blk (4) inodes, 2 data blocks (one for root)
	format_disk();
	test_init_mount();
	prt_fs();
	prt_table();

	printf("\n\n\n\n END OF INIT \n\n\n\n");
	//printf("d entry size:%d, num:%d\n", sizeof(dir_entry_t), fs.spb.size/sizeof(dir_entry_t));

	int next_inode = extract_next_free_inode();
	create_file_at_inode(next_inode, "test_data_file/data1.txt");
	printf("\n\n\n\n END OF CREATE \n\n\n\n");

	append_file_at_inode(next_inode, "test_data_file/append2.txt");

	printf("\n\n\n\n END OF APPEND \n\n\n\n");


	char* readed_data = (char*)malloc(512);
	char* readed_data2 = (char*)malloc(5);

	readed_data[511] = '\0';
	readed_data2[4] = '\0';

	f_rewind(next_inode);
	f_read(next_inode, readed_data, 511);
	f_read(next_inode, readed_data2, 4);

	printf("\n\n\n\n END OF READ \n\n\n\n");

	printf("%s + %s\n", readed_data, readed_data2);
	free(readed_data);
	free(readed_data2);


	prt_table();


	free(fs.inodes);
	free(fs.table.open_files);
} 

//// HELPER FUNCS ///// 

