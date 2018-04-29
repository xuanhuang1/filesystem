#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

fs_attr_t fs;

void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size);
void set_root(inode_t* root);
void set_empty_inode(inode_t* empty_inode);
void prt_spb(spb_t spb);
void prt_root(inode_t root);
void prt_inode(inode_t);
void prt_data_region(spb_t t);

// fread and check written size
size_t read_f_with_fread();

// return the number of inodes indicated in spb
int get_inode_count(spb_t spb);

// set file system globals
void set_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255]);

void prt_fs();



// one spb, one blk inodes, 2 data block (one for root)
int test_create_disk(){
	int spb_size = 512;
	int block_size = 512;
	int inode_b_num = 1;
	int data_b_num = 4;

	int i_offset = 0;
	int d_offset = inode_b_num;
	int total_b_num = i_offset + inode_b_num + data_b_num; 

	FILE *ptr_opt = fopen("disk","w");
	printf("size inode:%ld spb:%ld\n", sizeof(inode_t), sizeof(spb_t));

	char* buffer = (char*)malloc(spb_size*total_b_num);
	memset(buffer, 0, spb_size*total_b_num);
	spb_t* spb = (spb_t*)buffer;
	inode_t* root = (inode_t*)(buffer+spb_size);

	set_spb(spb, i_offset, d_offset, block_size);
	set_root(root);

	prt_spb(*spb);
	prt_root(*root);


	// fill the rest with empty inodes
	int inode_count = get_inode_count(*spb);
	printf("%d except root\n", inode_count-1);
	for (int i=1; i<inode_count; i++){
		inode_t* empty_inode = (inode_t*)(buffer+spb_size*(1+i_offset)+i*sizeof(inode_t));
		set_empty_inode(empty_inode);
		if(i == inode_count-1) 
			empty_inode->next_free_inode = -1;
		else 
			empty_inode->next_free_inode = i+1;
		prt_inode(*empty_inode);
	}

	// connect all the free blocks
	for (int i = 1; i < data_b_num; ++i){
		int* data_b = (int*)(buffer+spb_size+d_offset*spb_size+i*sizeof(spb_size));
		if(i == data_b_num-1)
			*data_b = -1;
		else *data_b = i+1;
	}
	
	fwrite(buffer, spb_size*total_b_num, 1, ptr_opt);
	fclose(ptr_opt);
	free(buffer);

	return 1;
}

int test_init(){
	// read the size of file
	// init write buffer
	FILE *ptr_ipt = fopen("disk","r");
	fseek(ptr_ipt, 0L, SEEK_END);
	int sz = ftell(ptr_ipt);
	char *infile = (char*)malloc(sz+1);
	memset(infile, 0, sz);
	infile[sz] = '\0';
	rewind(ptr_ipt);


	printf("\nopen disk!\n");
	read_f_with_fread(infile, sizeof(char), sz, ptr_ipt);
	fclose(ptr_ipt);

	spb_t *out_spb = (spb_t*)infile;
	int blocksize = out_spb->size;

	char* inodes_region = (char*)malloc((out_spb->data_offset - out_spb->inode_offset)*blocksize);
	memcpy(inodes_region, infile + 512 + out_spb->inode_offset, (out_spb->data_offset - out_spb->inode_offset)*blocksize);

	inode_t *inodes = (inode_t*)inodes_region;
	inode_t *out_root = inodes;


	
	prt_spb(*out_spb);
	prt_root(*out_root);
	int inode_count = get_inode_count(*out_spb);
	for (int i = 0; i < inode_count-1; ++i){
		prt_inode(inodes[i+1]);
	}

	set_fs(*out_spb, SUPERUSER, inodes, inode_count, "DISK");

	free(infile);
	free(inodes_region);


	return 1;

}

/*
 find the inode by filename at given directory current_d
 check permission, if doesn’t match with flag return -1 and set errno
 if reading: if not found return error
 if writing/appending:
     if not found,  find the first empty entry in fs.table and create a f_entry_t obj
        assign a inode in fs.freeiHead
        set the new inode’s parent to current_d
        add the new inode’s inode number into current_d’s data region
     if no fs.freeiHead return error, else set new fs.freeiHead
 return the index of new file in fs.table when success
 */
int f_open(inode_t current_d, const char *filename, int flags);




int main(){
	// one spb, one blk (4) inodes, 2 data blocks (one for root)
	test_create_disk();
	test_init();
	prt_fs();

	free(fs.inodes);
} 

//// HELPER FUNCS ///// 

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

size_t read_f_with_fread(char* infile, int fsize, int count, FILE* ptr_ipt){
	size_t newLen = fread(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error reading\n");
	return newLen;
}

int get_inode_count(spb_t spb){
	int region_size = (spb.data_offset - spb.inode_offset)*spb.size;
	return region_size / sizeof(inode_t);
}

void set_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255]){
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
}

void prt_fs(){
	printf("\nfile system attributes:\n");
	prt_spb(fs.spb);
	prt_root(*(fs.root));
	int inode_count = get_inode_count(fs.spb);
	for (int i = 0; i < inode_count-1; ++i){
		prt_inode(fs.inodes[i+1]);
	}

}