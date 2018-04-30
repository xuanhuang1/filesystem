#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

extern fs_attr_t fs;

void prt_spb(spb_t spb){
	printf("spb:  \ttype:%d size:%d ioff:%d dataoff:%d freei:%d freeb:%d\n",
			spb.type, spb.size, spb.inode_offset, spb.data_offset,
			spb.free_inode, spb.free_block);
}

void prt_root(inode_t root){
	printf("root: \tdir:%d parent:%d children_num:%d nlink:%d dblocks[0]:%d size:%d\n",
			root.isdir, root.parent, root.children_num, root.nlink, root.dblocks[0], root.size);
}

void prt_inode(inode_t i){
	printf("inode:\tdir:%d parent:%d children_num:%d nlink:%d dblocks[0]:%d size:%d next free:%d\n",
			i.isdir, i.parent, i.children_num, i.nlink, i.dblocks[0], i.size, i.next_free_inode);
}


void prt_data_region(){
	printf("\nprint data region:\n");
	for (int i = 0; i < fs.data_block_num; ++i){
		char* data = get_data_block(i);
		int data_next = *((int*)data);
		printf("data block:%d next free:%d\n", i, data_next);
		free(data);
	}
}

void prt_table(){
	printf("\nprint file entry table:\n");
	for (int i = 0; i < fs.table.length; ++i){
		assert (fs.table.open_files[i].ind > -1);
		printf("fs.table.open_files[%d]: inode %d\n\t", i, fs.table.open_files[i].ind);
		prt_inode(fs.inodes[fs.table.open_files[i].ind]);
	}

}

void prt_fs(){
	printf("\nfile system attributes:\n");
	prt_spb(fs.spb);
	prt_root(*(fs.root));
	int inode_count = get_inode_count(fs.spb);
	for (int i = 0; i < inode_count-1; ++i){
		prt_inode(fs.inodes[i+1]);
	}
	prt_data_region();

}


int printDB(inode_t inode, int block_count){
	//printf("print db\n");
  	
	for (int j = 0; j < N_DBLOCKS; ++j){
		char* buffer = get_data_block(inode.dblocks[j]);
		for (int i = 0; i < fs.spb.size; ++i)
			printf("%c", buffer[i]);
		
		free(buffer);
		block_count--;
		if(!block_count){
			return 0;
		}
	}
	return block_count;
}


void prt_file_data(int inode_indx){
	printf("\n print file date inodes[%d]:\n", inode_indx);
	inode_t the_inode = fs.inodes[inode_indx];
	int block_count = the_inode.size/fs.spb.size;
	if(fs.spb.size*block_count < the_inode.size) block_count++;
	printf("the inode size:%d\n", the_inode.size);
	if(the_inode.size)
		block_count = printDB(the_inode, block_count);
}