#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern fs_attr_t fs;

int delete_file_by_inode(int inode_index){
	int result = trunc_file(&fs.inodes[inode_index]);
	fs.inodes[inode_index].nlink = 0;
	free_this_inode(inode_index);
	return result;
}

int trunc_file(inode_t *inode){
	int size_to_trunc = inode->size;
	if(!size_to_trunc) return SUCCESS;
	for (int i = 0; i < N_DBLOCKS; ++i){
		free_this_block(inode->dblocks[i]);
		size_to_trunc -= fs.spb.size;
		if(size_to_trunc < 1){ // no more blocks to trunc
			inode->size = 0;
			return SUCCESS;
		}
		inode->dblocks[i] = -1;
	}

	int table_len = fs.spb.size/sizeof(int);
	for (int i = 0; i < N_IBLOCKS; ++i){
		int *table = (int*)get_one_data_block(inode->iblocks[i], 0, fs.spb.size);
		for (int j = 0; j < table_len; ++j){
			free_this_block(table[j]);
			size_to_trunc -= fs.spb.size;
			if(size_to_trunc < 1){ // no more blocks to trunc
				inode->size = 0;

				return SUCCESS;
			}
		}
		free(table);
		free_this_block(inode->iblocks[i]);
	}


	if(size_to_trunc){
		int *table2 = (int*)get_one_data_block(inode->i2block, 0, fs.spb.size);
		for (int i = 0; i < table_len; ++i){
			int *table = (int*)get_one_data_block(table2[i], 0, fs.spb.size);
			for (int j = 0; j < table_len; ++j){
				free_this_block(table[j]);
				size_to_trunc -= fs.spb.size;
				if(size_to_trunc < 1){ // no more blocks to trunc
					inode->size = 0;

					return SUCCESS;
				}
			}
			free(table);
			free_this_block(table2[i]);
		}
		free(table2);
		free_this_block(inode->i2block);
	}

	if(size_to_trunc){
		int *table3 = (int*)get_one_data_block(inode->i3block, 0, fs.spb.size);
		for(int k = 0; k < table_len; ++k){
			int *table2 = (int*)get_one_data_block(table3[k], 0, fs.spb.size);
			for (int i = 0; i < table_len; ++i){
				int *table = (int*)get_one_data_block(table2[i], 0, fs.spb.size);
				for (int j = 0; j < table_len; ++j){
					free_this_block(table[j]);
					size_to_trunc -= fs.spb.size;
					if(size_to_trunc < 1){ // no more blocks to trunc
						inode->size = 0;
						return SUCCESS;
					}
				}
				free(table);
				free_this_block(table2[i]);
			}
			free(table2);
			free_this_block(table3[k]);
		}
		free(table3);
		free_this_block(inode->i3block);
	}

	return FAIL;

}

