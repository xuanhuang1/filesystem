#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;

inode_t* search_file_in_one_block(int index, char* filename, int* count){
	char* buffer = get_data_block(index, 0, fs.spb.size);
	dir_entry_t *entris = (dir_entry_t*)buffer;
	for (int j = 0; j < fs.spb.size/sizeof(dir_entry_t); ++j){
		if(strcmp(entris[j].name, filename)==0){
			free(buffer);
			return &(fs.inodes[entris[j].ind]);
		}
		(*count)--;
		if(!(*count)){
			free(buffer);
			return NULL;
		}
	}
	free(buffer);
	return NULL;
}

inode_t* search_file_in_dir(int dir_index, char* filename){
	inode_t the_dir = fs.inodes[dir_index];

	assert (dir_index > -1);
	assert (the_dir.nlink > 0);
	assert (the_dir.isdir == 1);

	int num_to_search = the_dir.children_num;
	if(num_to_search < 1){
		printf("try to search file in an empty directory %d\n", dir_index);
		return NULL;
	}

	// check dblocks
	for (int i = 0; i < N_DBLOCKS; ++i){
		inode_t* temp = search_file_in_one_block(the_dir.dblocks[i], filename, &num_to_search);
		if((temp)||(!num_to_search))  return temp;
	}
	return NULL;
}

int write_dblock(inode_t* inode, int write_size, char* in_buff){
	int bytes_left = write_size;
	for (int i = 0; i < N_DBLOCKS; ++i){
		int size_write_to = fs.spb.size;
		if(bytes_left < size_write_to) size_write_to = bytes_left;
		char* buffer = (char*)malloc(size_write_to+1);
		memset(buffer, 0, size_write_to);
		buffer[size_write_to] = '\0';

		//printf("the buffer after set:%s\n", buffer);
		
		memcpy(buffer, in_buff+i*fs.spb.size, size_write_to);
		printf("copying %d to %d\n", i*fs.spb.size, i*fs.spb.size+size_write_to-1);
		//printf("the buffer:%s\n", buffer);

		write_data_block(inode->dblocks[i], buffer, 0, size_write_to);

		bytes_left -= size_write_to;
		free(buffer);
		printf("------bytes_left: %d------\n", bytes_left);
		if(!bytes_left) return write_size;
	}
	return write_size - bytes_left;
}

int write_file(inode_t* inode, char* buff, int len, int flag){
	printf("original file: len buff:%d\n", len);
	//printf("***%s***\n", buff);
	int size_to_write = len;
	write_dblock(inode, size_to_write, buff);
	inode->size = len;
	//printf("size %d\n", inode->size);
	return 1;
}
