#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H
#include "../fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prt_spb(spb_t spb);
void prt_root(inode_t root);
void prt_inode(inode_t);
void prt_data_region();
void prt_fs();

void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size);
void set_root(inode_t* root);
void set_empty_inode(inode_t* empty_inode);

// set file system globals
void set_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255], int db_num);
// fread and check written size
size_t read_f_with_fread(char* infile, int fsize, int count, FILE* ptr_ipt);
// return the number of inodes indicated in spb
int get_inode_count(spb_t spb);
char* get_data_block(char diskname[255], int data_index, spb_t spb);

#endif