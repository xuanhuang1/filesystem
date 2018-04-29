#include "helper_funcs.h"



extern fs_attr_t fs;

size_t read_f_with_fread(char* infile, int fsize, int count, FILE* ptr_ipt){
	size_t newLen = fread(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error reading file\n");
	return newLen;
}

char* get_data_block(char diskname[255], int data_index, spb_t spb){
	FILE *ptr_ipt = fopen("disk","r");
	char* buffer = (char*)malloc(sizeof(spb.size));
	int data_offset_on_disk = spb.size+spb.size*(spb.data_offset+data_index);
	printf("data_off_on_disk %d\n", data_offset_on_disk);
	fseek(ptr_ipt, data_offset_on_disk, SEEK_SET);
	read_f_with_fread(buffer, sizeof(char), sizeof(spb.size), ptr_ipt);
	fclose(ptr_ipt);
	return buffer;
}


int get_inode_count(spb_t spb){
	int region_size = (spb.data_offset - spb.inode_offset)*spb.size;
	return region_size / sizeof(inode_t);
}



void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size){
	spb->type = 0;
	spb->size = block_size;
	spb->inode_offset = i_offset;
	spb->data_offset = d_offset;
	spb->free_inode = 1;
	spb->free_block = 1;
}

void set_root(inode_t* root){
	root->parent = -1;
	root->children_num = 0;
	root->nlink = 1;
	root->dblocks[0] = 0;
	root->size = 512;
}

void set_empty_inode(inode_t* empty_inode){
	empty_inode->parent = -1;
	empty_inode->children_num = 0;
	empty_inode->nlink = 0;
	empty_inode->dblocks[0] = -1;
	empty_inode->size = -1;
}


void set_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255], int db_num){
	fs.spb = spb;
	fs.users[SUPERUSER] = 0;
	fs.users[USER] = 1;
	fs.user = user_in;


	fs.inodes = (inode_t*)malloc(sizeof(inode_t)*inodes_arr_len);
	for (int i = 0; i < inodes_arr_len; ++i)
		fs.inodes[i] = inodes_arr[i];

	fs.root = &fs.inodes[0];
	fs.shell_d = fs.root;
	fs.freeiHead = &inodes_arr[spb.free_inode];
	fs.free_block_head = spb.free_block;

	int c = 0;
  	while ((name[c] != '\0') && (c < 255 -1)) {
    	fs.diskname[c] = name[c];
    	c++;
  	}
	fs.diskname[c] = '\0';
	fs.fs_num = 0;
	fs.data_block_num = db_num;
}