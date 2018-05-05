#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;

int read_dblock(inode_t* inode, int read_size, int start_db_i, int offset, void* out_buff){
	int bytes_read = 0;
	for (int i = start_db_i; i < N_DBLOCKS; ++i){
		int size_read_to = fs.spb.size;
		int this_read = 0;

		if(i == start_db_i){size_read_to -= offset;}

		if(read_size - bytes_read < size_read_to) size_read_to = read_size - bytes_read;
		printf("\nreading into buffer at %d to %d\n", bytes_read, bytes_read+size_read_to-1);
		
		char* buffer = (char*)malloc(size_read_to+1);
		buffer[size_read_to] = '\0';

		if(i == start_db_i)
			this_read = read_one_data_block(inode->dblocks[i], buffer, offset, size_read_to);
		else
			this_read = read_one_data_block(inode->dblocks[i], buffer, 0, size_read_to);
		//printf("%s\n", buffer);
		memcpy(out_buff+bytes_read, buffer, size_read_to);
		free(buffer);

		bytes_read+= this_read;

		printf("------bytes_left: %d------\n", read_size - bytes_read);
		if(read_size == bytes_read) return read_size;
	}
	return bytes_read;
		
}


int read_file_by_inode(f_entry_t* the_file_entry, void* out_buff, int len){
	int size_to_read = len;
	int bytes_read = 0;
	inode_t* inode = &fs.inodes[the_file_entry->ind];
	if(inode->size < len + the_file_entry->offset) 
		size_to_read = inode->size - the_file_entry->offset;

	printf("Start reading, len buff:%d\n", len);
	if(the_file_entry->offset < N_DBLOCKS*fs.spb.size){
		int db_index = the_file_entry->offset/fs.spb.size;
		int offset = the_file_entry->offset%(fs.spb.size);
		printf("\tReading from dbindex:%d offset:%d\n", db_index, offset);

		assert(db_index < 10);
		assert(offset < fs.spb.size);
		bytes_read += read_dblock(inode, size_to_read, db_index, offset, out_buff);
	}
	the_file_entry->offset += bytes_read;
	return bytes_read;
}