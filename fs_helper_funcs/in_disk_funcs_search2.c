#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;




char* search_name_in_one_block(int index, int file_index, int* count){
	dir_entry_t *entris = (dir_entry_t*)get_one_data_block(index, 0, fs.spb.size);
	printf("search data bolck %d\n", index);
	for (int j = 0; j < fs.spb.size/sizeof(dir_entry_t); ++j){
		printf("entris index:%d and file_index:%d\n", entris[j].ind, file_index);
		if(entris[j].ind == file_index){
			//printf("inode %d\n", entris[j].ind);
			char *ret = (char*)malloc(NAMELEN*sizeof(char));
			memset(ret, 0, NAMELEN);
			memcpy(ret, entris[j].name, NAMELEN);
			free(entris);
			return ret;
		}
		(*count)--;
		if(!(*count)){
			free(entris);
			printf("use up counts\n");
			return NULL;
		}
	}
	free(entris);
	return NULL;
}

char* search_name_in_dir(int dir_index, int file_index){
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
		char* temp = search_name_in_one_block(the_dir.dblocks[i], file_index, &num_to_search);
		if((!num_to_search)|| (temp!=NULL) )  return temp;
	}

	for (int i = 0; i < N_IBLOCKS; ++i){
		int *table = (int*)get_one_data_block(the_dir.iblocks[i], 0, fs.spb.size);
		int table_len = fs.spb.size/sizeof(int);
		for (int j = 0; j < table_len; ++j){
			printf("idirect %d\n", table[j]);
			char* temp = search_name_in_one_block(table[j], file_index, &num_to_search);
			if((!num_to_search)|| (temp!=NULL)){ 
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
				char* temp = search_name_in_one_block(table[j], file_index, &num_to_search);
				if((!num_to_search)|| (temp!=NULL)){ 
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
					char* temp = search_name_in_one_block(table[j], file_index, &num_to_search);
					if((!num_to_search)|| (temp!=NULL)){ 
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

	return NULL;
}
