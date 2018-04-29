#include "../fs.h"
#include "helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern fs_attr_t fs;

void prt_spb(spb_t spb){
	printf("spb:    \ttype:%d size:%d ioff:%d dataoff:%d freei:%d freeb:%d\n",
			spb.type, spb.size, spb.inode_offset, spb.data_offset,
			spb.free_inode, spb.free_block);
}

void prt_root(inode_t root){
	printf("root:   \tparent:%d children_num:%d nlink:%d dblocks[0]:%d size:%d\n",
			root.parent, root.children_num, root.nlink, root.dblocks[0], root.size);
}

void prt_inode(inode_t i){
	printf("inode:   \tparent:%d children_num:%d nlink:%d dblocks[0]:%d size:%d next free:%d\n",
			i.parent, i.children_num, i.nlink, i.dblocks[0], i.size, i.next_free_inode);
}


void prt_data_region(){
	printf("print data region:\n");
	for (int i = 0; i < fs.data_block_num; ++i){
		char* data = get_data_block(fs.diskname, i, fs.spb);
		int data_next = *((int*)data);
		printf("data block:%d next free:%d\n", i, data_next);
		free(data);
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