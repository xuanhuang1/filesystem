#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;

int write_dblock(inode_t* inode, int write_size, int start_db_i, int offset, void* in_buff){
	int bytes_written = 0;
	int j = 0;
	for (int i = start_db_i; i < N_DBLOCKS; ++i){
		int size_write_to = fs.spb.size;
		if(i == start_db_i){size_write_to -= offset;}

		// get new block if exceed current size 
		if(inode->size < fs.spb.size*i+1 ) 
			inode->dblocks[i] = extract_next_free_block();

		if(write_size - bytes_written < size_write_to) size_write_to = write_size - bytes_written;
		char* buffer = (char*)malloc(size_write_to+1);
		memset(buffer, 0, size_write_to);
		buffer[size_write_to] = '\0';

		//printf("the buffer after set:%s\n", buffer);
		
		memcpy(buffer, in_buff+j, size_write_to);
		printf("\tcopying %d to %d\n", j, j+size_write_to-1);
		//printf("the buffer:%s\n", buffer);

		if(i == start_db_i)
		bytes_written += write_one_data_block(inode->dblocks[i], buffer, offset, size_write_to);
		else
		bytes_written += write_one_data_block(inode->dblocks[i], buffer, 0, size_write_to);

		free(buffer);
		printf("------bytes_left: %d------\n", write_size - bytes_written);

		if(write_size == bytes_written) return write_size;
		j+= size_write_to;

	}
	return bytes_written;
}

int write_file_by_inode(f_entry_t* the_file_entry, void* buff, int len){
	printf("\toriginal file: len buff:%d\n", len);
	//printf("***%s***\n", buff);
	int size_to_write = len;
	int bytes_written = 0;
	inode_t* inode = &fs.inodes[the_file_entry->ind];

	// set offset acording to flags
	if(the_file_entry->mode == OPEN_APPEND){
		the_file_entry->offset = inode->size;
	}else if(the_file_entry->mode == OPEN_W){
		the_file_entry->offset = 0;
	}else if(the_file_entry->mode == OPEN_RW){
		// let offset stays
	}else{printf("WRONGF IN f_write\n");}

	if(the_file_entry->offset < N_DBLOCKS*fs.spb.size){
		int len_to_write = size_to_write;
		if(len_to_write > N_DBLOCKS*fs.spb.size - the_file_entry->offset)
			len_to_write = N_DBLOCKS*fs.spb.size - the_file_entry->offset;
		int db_index = the_file_entry->offset/fs.spb.size;
		int offset = the_file_entry->offset%(fs.spb.size);
		printf("\tDB: writing:%d bytes in db, ", len_to_write);
		printf("starting at dbindex:%d offset:%d\n", db_index, offset);

		assert(db_index < 10);
		assert(offset < fs.spb.size);
		bytes_written += write_dblock(inode, size_to_write, db_index, offset, buff);
		size_to_write -= bytes_written;
	}
	printf("\ttotal len left:%d\n", size_to_write);
	//write_dblock(inode, size_to_write, 0, 0, buff);
	
	inode->size =  the_file_entry->offset+bytes_written;
	the_file_entry->offset += bytes_written;
	
	return bytes_written;
}
