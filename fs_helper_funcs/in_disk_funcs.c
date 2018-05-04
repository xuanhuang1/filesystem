#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;

inode_t* search_file_in_one_block(int index, char* filename, int* count){
	char* buffer = get_one_data_block(index, 0, fs.spb.size);
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

int write_dblock(inode_t* inode, int write_size, int start_db_i, int offset, char* in_buff){
	int bytes_left = write_size;
	int j = 0;
	for (int i = start_db_i; i < N_DBLOCKS; ++i){
		int size_write_to = fs.spb.size;
		if(i == start_db_i){size_write_to -= offset;}

		if(offset == 0 ) inode->dblocks[i] = extract_next_free_block();

		if(bytes_left < size_write_to) size_write_to = bytes_left;
		char* buffer = (char*)malloc(size_write_to+1);
		memset(buffer, 0, size_write_to);
		buffer[size_write_to] = '\0';

		//printf("the buffer after set:%s\n", buffer);
		
		memcpy(buffer, in_buff+j, size_write_to);
		printf("\tcopying %d to %d\n", j, j+size_write_to-1);
		//printf("the buffer:%s\n", buffer);

		if(i == start_db_i)
		write_one_data_block(inode->dblocks[i], buffer, offset, size_write_to);
		else
		write_one_data_block(inode->dblocks[i], buffer, 0, size_write_to);

		bytes_left -= size_write_to;
		free(buffer);
		printf("------bytes_left: %d------\n", bytes_left);
		if(!bytes_left) return write_size;
		j+= size_write_to;
	}
	return write_size - bytes_left;
}

int write_file_by_inode(f_entry_t* the_file_entry, char* buff, int len){
	printf("\toriginal file: len buff:%d\n", len);
	//printf("***%s***\n", buff);
	int size_to_write = len;
	inode_t* inode = &fs.inodes[the_file_entry->ind];
	if(the_file_entry->mode == OPEN_APPEND){
		the_file_entry->offset = inode->size;
	}else if(the_file_entry->mode == OPEN_W){
		the_file_entry->offset = 0;
		printf("open_w\n");
	}

	if(the_file_entry->offset < N_DBLOCKS*fs.spb.size){
		int db_index = the_file_entry->offset/fs.spb.size;
		int offset = the_file_entry->offset%(fs.spb.size);
		printf("\tindex:%d offset:%d\n", db_index, offset);

		assert(db_index < 10);
		assert(offset < fs.spb.size);
		write_dblock(inode, size_to_write, db_index, offset, buff);
	}
	//write_dblock(inode, size_to_write, 0, 0, buff);
	if(the_file_entry->mode == OPEN_APPEND)
		inode->size =  the_file_entry->offset+len;
	the_file_entry->offset += len;
	//printf("size %d\n", inode->size);
	return SUCCESS;
}

int read_dblock(inode_t* inode, int read_size, int start_db_i, int offset, char* out_buff){
	int bytes_left = read_size;
	int j = 0;
	for (int i = start_db_i; i < N_DBLOCKS; ++i){
		int size_read_to = fs.spb.size;
		if(i == start_db_i){size_read_to -= offset;}

		if(bytes_left < size_read_to) size_read_to = bytes_left;
		printf("\nreading into buffer at %d to %d\n", j, j+size_read_to-1);
		char* buffer;

		if(i == start_db_i)
			buffer = get_one_data_block(inode->dblocks[i], offset, size_read_to);
		else
			buffer = get_one_data_block(inode->dblocks[i], 0, size_read_to);
		printf("%s\n", buffer);
		memcpy(out_buff+j, buffer, size_read_to);


		bytes_left -= size_read_to;
		free(buffer);
		printf("------bytes_left: %d------\n", bytes_left);
		if(!bytes_left) return read_size;
		j+= size_read_to;
	}
	return read_size - bytes_left;
		
}


int read_file_by_inode(f_entry_t* the_file_entry, char* out_buff, int len){
	int size_to_read = len;
	inode_t* inode = &fs.inodes[the_file_entry->ind];
	if(inode->size < len + the_file_entry->offset) 
		size_to_read = inode->size - the_file_entry->offset;

	if(the_file_entry->offset < N_DBLOCKS*fs.spb.size){
		int db_index = the_file_entry->offset/fs.spb.size;
		int offset = the_file_entry->offset%(fs.spb.size);
		printf("\tindex:%d offset:%d\n", db_index, offset);

		assert(db_index < 10);
		assert(offset < fs.spb.size);
		read_dblock(inode, size_to_read, db_index, offset, out_buff);

	}
	the_file_entry->offset += size_to_read;
	return SUCCESS;
	/*int size_to_read = len;
	int db_index = inode->size/fs.spb.size;
	int offset = inode->size%(fs.spb.size);
	printf("\tindex:%d offset:%d\n", db_index, offset);
	assert(db_index < 10);
	assert(offset < fs.spb.size);

	int byte_count = inode->size;
	printf("the inode size:%d\n", inode->size);
	return byte_count;*/
}