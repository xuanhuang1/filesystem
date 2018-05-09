#include "helper_funcs.h"
#include <assert.h>
#include <string.h>



extern fs_attr_t fs;


void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size, int freei, int freeb){
	spb->type = 0;
	spb->size = block_size;
	spb->inode_offset = i_offset;
	spb->data_offset = d_offset;
	spb->free_inode = freei;
	spb->free_block = freeb;
}

void set_root(inode_t* root){
	root->parent = -1;
	root->isdir = 1;
	root->children_num = 0;
	root->nlink = 1;
	root->dblocks[0] = -1;
	root->size = 0;
}

void set_empty_inode(inode_t* empty_inode){
	empty_inode->parent = -1;
	empty_inode->children_num = 0;
	empty_inode->nlink = 0;
	empty_inode->dblocks[0] = -1;
	empty_inode->size = -1;
}

// get the total number of inodes
int get_inode_count(spb_t spb){
	int region_size = (spb.data_offset - spb.inode_offset)*spb.size;
	return region_size / sizeof(inode_t);
}

// load all globals into the gloabal struct
void load_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255], int db_num){
	fs.spb = spb;
	fs.u_gid[SUPERUSER] = 1;
	fs.u_gid[USER] = 1;
	fs.user = user_in;


	fs.inodes = (inode_t*)malloc(sizeof(inode_t)*inodes_arr_len);
	for (int i = 0; i < inodes_arr_len; ++i)
		fs.inodes[i] = inodes_arr[i];

	fs.root = 0;
	fs.shell_d = 0;
	fs.freeiHead = spb.free_inode;
	fs.free_block_head = spb.free_block;

	int c = 0;
  	while ((name[c] != '\0') && (c < 255 -1)) {
    	fs.diskname[c] = name[c];
    	c++;
  	}
	fs.diskname[c] = '\0';
	//printf("disk name %s is disk?%d \n", fs.diskname, strcmp(fs.diskname,"disk")==0);
	fs.fs_num = 0;
	fs.data_block_num = db_num;
}