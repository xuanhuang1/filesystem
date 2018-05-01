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
void prt_table();

int printDB(inode_t inode, int block_count);
void prt_file_data(int inode_indx);

void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size);
void set_root(inode_t* root);
void set_empty_inode(inode_t* empty_inode);

// return the number of inodes indicated in spb
int get_inode_count(spb_t spb);
int get_file_size(FILE *ptr_ipt, char** infile);

// set file system globals
void load_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255], int db_num);

// fread and fwrite related
size_t read_f_into_buffer(char* infile, int fsize, int count, FILE* ptr_ipt);
size_t write_f_from_buffer(char* infile, int fsize, int count, FILE* ptr_ipt);
char* get_data_block(int data_index, int offset, int length);
char* write_data_block(int data_index, char* buffer, int offset, int length);



int format_disk();
inode_t* search_file_in_indir(inode_t the_dir, char* filename);
int write_file(inode_t* inode, char* buff, int len, int flag);

#endif