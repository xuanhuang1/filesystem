#include "fs.h"
#include "fs_helper_funcs/helper_funcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern fs_attr_t fs;


// one spb, one blk inodes, 2 data block (one for root)
int format_disk(){
	int spb_size = 512;
	int block_size = 512;
	int inode_b_num = 1;
	int data_b_num = 6;

	int i_offset = 0;
	int d_offset = inode_b_num;
	int total_b_num = 1+i_offset + inode_b_num + data_b_num; 

	FILE *ptr_opt = fopen("disk","w");
	//printf("size inode:%ld spb:%ld\n", sizeof(inode_t), sizeof(spb_t));

	char* buffer = (char*)malloc(spb_size*total_b_num);
	memset(buffer, 0, spb_size*total_b_num);
	spb_t* spb = (spb_t*)buffer;
	inode_t* root = (inode_t*)(buffer+spb_size);


	set_spb(spb, i_offset, d_offset, block_size, 1, 0);
	set_root(root);
	int inode_count = get_inode_count(*spb);

	prt_spb(*spb);
	prt_root(*root);
	//printf("inode count:%d\n", inode_count);

	// fill the rest with empty inodes
	//printf("%d except root\n", inode_count-1);
	for (int i=1; i<inode_count; i++){
		inode_t* empty_inode = (inode_t*)(buffer+spb_size*(1+i_offset)+i*sizeof(inode_t));
		set_empty_inode(empty_inode);
		if(i == inode_count-1) 
			empty_inode->next_free_inode = -1;
		else 
			empty_inode->next_free_inode = i+1;
		//prt_inode(*empty_inode);
	}

	// connect all the free blocks
	for (int i = 0; i < data_b_num; ++i){
		int* data_b = (int*)(buffer+spb_size+d_offset*spb_size+i*spb_size);
		if(i == data_b_num-1)
			*data_b = -1;
		else *data_b = i+1;
		//printf("%d: data b offset:%d next:%d\n", i, spb_size+d_offset*spb_size+i*spb_size, *data_b);

	}
	//printf("sz:%d, data b num %d\n",spb_size*total_b_num, data_b_num);
	
	fwrite(buffer, spb_size*total_b_num, 1, ptr_opt);
	fclose(ptr_opt);
	free(buffer);

	return 1;
}


