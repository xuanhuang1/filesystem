#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;


int remove_last_entry_in_dir(int dir_index, dir_entry_t *ret_entry){
	inode_t the_dir = fs.inodes[dir_index];
	if(the_dir.isdir == FALSE){
		printf("Trying to remove last entry in a non dir file %d!\n",dir_index);
		return FAIL;
	}
	int size_left = the_dir.size;
	if(size_left < fs.spb.size*N_DBLOCKS){
		int dbindex = size_left/fs.spb.size;
		int offset = size_left%fs.spb.size;

		dir_entry_t *entry = (dir_entry_t*)get_one_data_block(fs.inodes[dir_index].dblocks[dbindex], offset, sizeof(dir_entry_t));
		
		memset(ret_entry, 0, NAMELEN);
		memcpy(ret_entry, ret_entry->name, NAMELEN);
		ret_entry->ind = entry->ind;
		entry->ind = EMPTY_ENTRY;
		entry->name[0] = '\0';
		return SUCCESS;
	}
	int table_len = fs.spb.size/sizeof(dir_entry_t);
	size_left -= fs.spb.size*N_DBLOCKS;

	if(size_left < fs.spb.size*table_len*N_IBLOCKS){
		int ibindex = size_left/(fs.spb.size*table_len);	// which iblocks
		int i1_table_offset = size_left/(fs.spb.size);		// where in i1 table
		int data_block_offset = size_left%fs.spb.size;		// where in data block

		int *i1_table_num = (int*)get_one_data_block(fs.inodes[dir_index].iblocks[ibindex], i1_table_offset, sizeof(int));
		dir_entry_t *entry = (dir_entry_t*)get_one_data_block(*i1_table_num, data_block_offset, sizeof(dir_entry_t));

		memset(ret_entry, 0, NAMELEN);
		memcpy(ret_entry, ret_entry->name, NAMELEN);
		ret_entry->ind = entry->ind;
		entry->ind = EMPTY_ENTRY;
		entry->name[0] = '\0';
		return SUCCESS;
	}

	return FAIL;

}

int remove_file_in_one_block(int index, char* filename, int* count);

int remove_file_in_dir(int dir_index, char* filename);
