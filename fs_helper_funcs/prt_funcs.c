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
	printf("\n=====================\n");
	printf("print data region:\n");
	printf("=====================\n");

	for (int i = 0; i < fs.data_block_num; ++i){
		char* data = get_one_data_block(i, 0, fs.spb.size);
		int data_next = *((int*)data);
		printf("data block:%d next free:%d\n", i, data_next);
		free(data);
	}

}

void prt_table(){
	printf("\n=======================\n");

	printf("print file entry table:\n");

	printf("=======================\n");
	for (int i = 0; i < fs.table.length; ++i){
		assert (fs.table.open_files[i].ind > -1);
		printf("fs.table.open_files[%d]: inode %d\n\t", i, fs.table.open_files[i].ind);
		prt_inode(fs.inodes[fs.table.open_files[i].ind]);
	}

}

void prt_fs(){
	printf("\n=====================\n");

	printf("file system attributes:\n");
	printf("=======================\n");

	prt_spb(fs.spb);
	prt_root(*(fs.root));
	int inode_count = get_inode_count(fs.spb);
	for (int i = 0; i < inode_count-1; ++i){
		prt_inode(fs.inodes[i+1]);
	}
	prt_data_region();
}


int printDB(inode_t inode, int byte_count){
	//printf("print db\n");
  	
	FILE *ptr_ipt = fopen("dataout.txt","w");
	for (int j = 0; j < N_DBLOCKS; ++j){
		int buffer_len = fs.spb.size;
		if(byte_count < fs.spb.size) buffer_len = byte_count;

		char* buffer = get_one_data_block(inode.dblocks[j], 0, buffer_len);
		fwrite(buffer, buffer_len, 1, ptr_ipt);
		byte_count -= buffer_len;
		free(buffer);
		if(!byte_count) {
			fclose(ptr_ipt);
			return byte_count;
		}

	}
	fclose(ptr_ipt);

	return byte_count;
}


void prt_file_data(int inode_indx){
	printf("\n=====================\n");
	printf("print file date inodes[%d]:\n", inode_indx);
	printf("======================\n");

	inode_t the_inode = fs.inodes[inode_indx];
	int byte_count = the_inode.size;
	printf("the inode size:%d\n", the_inode.size);
	if(the_inode.size)
		byte_count = printDB(the_inode, byte_count);

}