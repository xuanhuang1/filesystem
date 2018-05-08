#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define IPTR_NUM 4
enum {B_OFF, T1_OFF, T2_OFF, T3_OFF};
extern fs_attr_t fs;


void get_4_offsets_by_size(int size, int offsets[IPTR_NUM]){
	int table_len = fs.spb.size/sizeof(int);
	int size_left = size;
	offsets[T3_OFF] = size_left/(fs.spb.size*table_len*table_len);
	size_left -= offsets[T3_OFF]*(fs.spb.size*table_len*table_len);

	offsets[T2_OFF] = size_left/(fs.spb.size*table_len);
	size_left -= offsets[T2_OFF]*(fs.spb.size*table_len);

	offsets[T1_OFF] = size_left/(fs.spb.size);
	size_left -= offsets[T1_OFF]*(fs.spb.size);	

	offsets[B_OFF] = size_left;
}

int remove_last_entry_in_dir(int dir_index, dir_entry_t *ret_entry){
	inode_t the_dir = fs.inodes[dir_index];
	if(the_dir.isdir == FALSE){
		printf("Trying to remove last entry in a non dir file %d!\n",dir_index);
		return FAIL;
	}
	int size_left = the_dir.size - sizeof(dir_entry_t);
	if(size_left < 0){
		printf("Trying to remove last entry in a empty dir file %d!\n",dir_index);
		return FAIL;
	}
	int offsets[IPTR_NUM];

	if(size_left < fs.spb.size*N_DBLOCKS){
		get_4_offsets_by_size(size_left, offsets);

		dir_entry_t *entry = (dir_entry_t*)get_one_data_block(fs.inodes[dir_index].dblocks[offsets[T1_OFF]], offsets[B_OFF], sizeof(dir_entry_t));
		
		memset(ret_entry, 0, NAMELEN);
		memcpy(ret_entry->name, entry->name, NAMELEN);
		ret_entry->ind = entry->ind;
		printf("Last entry found in DB[%d]off=%d: ind:%d %s\n", offsets[T1_OFF], offsets[B_OFF], ret_entry->ind, ret_entry->name);
		free(entry);
		//printf("Now last entry in DB %d %s\n", entry->ind, entry->name);

		return SUCCESS;
	}

	int table_len = fs.spb.size/sizeof(int);
	size_left -= fs.spb.size*N_DBLOCKS;

	if(size_left < fs.spb.size*table_len*N_IBLOCKS){
		get_4_offsets_by_size(size_left, offsets);

		int *i1_table_num = (int*)get_one_data_block(fs.inodes[dir_index].iblocks[offsets[T2_OFF]], offsets[T1_OFF], sizeof(int));
		dir_entry_t *entry = (dir_entry_t*)get_one_data_block(*i1_table_num, offsets[B_OFF], sizeof(dir_entry_t));

		memset(ret_entry, 0, NAMELEN);
		memcpy(ret_entry->name, entry->name, NAMELEN);
		ret_entry->ind = entry->ind;
		free(entry);

		return SUCCESS;
	}

	size_left -= fs.spb.size*table_len*N_IBLOCKS;

	if(size_left < fs.spb.size*table_len*table_len){
		get_4_offsets_by_size(size_left, offsets);

		int *i2_table_num = (int*)get_one_data_block(fs.inodes[dir_index].i2block, offsets[T2_OFF], sizeof(int));
		int *i1_table_num = (int*)get_one_data_block(*i2_table_num, offsets[T1_OFF], sizeof(int));
		dir_entry_t *entry = (dir_entry_t*)get_one_data_block(*i1_table_num, offsets[B_OFF], sizeof(dir_entry_t));
		
		memset(ret_entry, 0, NAMELEN);
		memcpy(ret_entry->name, entry->name, NAMELEN);
		ret_entry->ind = entry->ind;
		free(entry);

		return SUCCESS;
	}

	size_left -= fs.spb.size*table_len*table_len;


	if(size_left < fs.spb.size*table_len*table_len*table_len){
		get_4_offsets_by_size(size_left, offsets);

		int *i3_table_num = (int*)get_one_data_block(fs.inodes[dir_index].i3block, offsets[T3_OFF], sizeof(int));
		int *i2_table_num = (int*)get_one_data_block(*i3_table_num, offsets[T2_OFF], sizeof(int));
		int *i1_table_num = (int*)get_one_data_block(*i2_table_num, offsets[T1_OFF], sizeof(int));
		dir_entry_t *entry = (dir_entry_t*)get_one_data_block(*i1_table_num, offsets[B_OFF], sizeof(dir_entry_t));
		
		memset(ret_entry, 0, NAMELEN);
		memcpy(ret_entry->name, entry->name, NAMELEN);
		ret_entry->ind = entry->ind;
		free(entry);
		
		return SUCCESS;
	}

	return FAIL;

}



int find_replace_dir_entry_in_one_block(int index, char* filename, int* count, dir_entry_t *entry_to_write){
	dir_entry_t *entris = (dir_entry_t*)get_one_data_block(index, 0, fs.spb.size);
	printf("search and replace dir entry in bolck %d\n", index);
	for (int j = 0; j < fs.spb.size/sizeof(dir_entry_t); ++j){
		printf("entris name:%s and filename:%s\n", entris[j].name, filename);
		if(strcmp(entris[j].name, filename)==0){
			printf("inode %d\n", entris[j].ind);
			int dir_entry_in_block_offset = j*sizeof(dir_entry_t);
			write_one_data_block(index, entry_to_write, dir_entry_in_block_offset, sizeof(dir_entry_t));
			//trunc_file(&fs.inodes[entris[j].ind]);
			//trunc_file(&fs.inodes[entris[j].ind]);
			//fs.inodes[entris[j].ind].nlink = 0;
			//free_this_inode(entris[j].ind);
			//delete_file_by_inode(entris[j].ind);
			int ret_val = entris[j].ind;
			free(entris);
			return ret_val;
		}
		(*count)--;
		if(!(*count)){
			free(entris);
			printf("use up counts\n");
			return EMPTY_ENTRY;
		}
	}
	free(entris);
	return EMPTY_ENTRY;
}

int find_replace_dir_entry(int dir_index, char* filename, dir_entry_t *entry_to_write){
	inode_t the_dir = fs.inodes[dir_index];

	assert (dir_index > -1);
	assert (the_dir.nlink > 0);
	assert (the_dir.isdir == 1);

	int num_to_search = the_dir.children_num;
	if(num_to_search < 1){
		printf("try to search and replace dir entry in an empty directory %d\n", dir_index);
		return EMPTY_ENTRY;
	}

	// check dblocks
	for (int i = 0; i < N_DBLOCKS; ++i){
		int temp = find_replace_dir_entry_in_one_block(the_dir.dblocks[i], filename, &num_to_search, entry_to_write);
		if((!num_to_search)|| (temp != EMPTY_ENTRY) )  return temp;
	}

	for (int i = 0; i < N_IBLOCKS; ++i){
		int *table = (int*)get_one_data_block(the_dir.iblocks[i], 0, fs.spb.size);
		int table_len = fs.spb.size/sizeof(int);
		for (int j = 0; j < table_len; ++j){
			printf("idirect %d\n", table[j]);
			int temp = find_replace_dir_entry_in_one_block(table[j], filename, &num_to_search, entry_to_write);
			if((!num_to_search)|| (temp != EMPTY_ENTRY)){ 
				free(table);
				return temp;
			}
		}
		free(table);
	}

	if(num_to_search){
		int *table2 = (int*)get_one_data_block(the_dir.i2block, 0, fs.spb.size);
		int table_len = fs.spb.size/sizeof(int);
		for (int i = 0; i < table_len; ++i){
			int* table = (int*)get_one_data_block(table2[i], 0, fs.spb.size);
			for (int j = 0; j < table_len; ++j){
				printf("2direct %d\n", table[j]);
				int temp = find_replace_dir_entry_in_one_block(table[j], filename, &num_to_search, entry_to_write);
				if((!num_to_search)|| (temp != EMPTY_ENTRY)){ 
					free(table);
					free(table2);
					return temp;
				}
			}
			free(table);
		}
		free(table2);
	}

	if(num_to_search){
		int *table3 = (int*)get_one_data_block(the_dir.i3block, 0, fs.spb.size);
		int table_len = fs.spb.size/sizeof(int);
		for (int k = 0; k < table_len; ++k){
			int* table2 = (int*)get_one_data_block(table3[k], 0, fs.spb.size);
			for (int i = 0; i < table_len; ++i){
				int* table = (int*)get_one_data_block(table2[i], 0, fs.spb.size);
				for (int j = 0; j < table_len; ++j){
					printf("3direct %d\n", table[j]);
					int temp = find_replace_dir_entry_in_one_block(table[j], filename, &num_to_search, entry_to_write);
					if((!num_to_search)|| (temp != EMPTY_ENTRY)){ 
						free(table);
						free(table2);
						free(table3);
						return temp;
					}
				}
				free(table);
			}
			free(table2);
		}
		free(table3);
	}

	assert (!num_to_search);



	if(the_dir.size > fs.spb.size*N_DBLOCKS)
		printf("LARGER THAN DB! SHOULDN'T HAPPEN NOW!\n");

	return -1;
}

