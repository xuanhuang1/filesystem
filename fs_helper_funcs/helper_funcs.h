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

void prt_file_data(int inode_indx);

void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size,int,int);
void set_root(inode_t* root);
void set_empty_inode(inode_t* empty_inode);

// return the number of inodes indicated in spb
int get_inode_count(spb_t spb);
int get_file_size(FILE *ptr_ipt, char** infile);
int extract_next_free_block();
int extract_next_free_inode();
void init_new_file_inode(int i, int parent, int nlink, int size, int uid, int gid, int in_time);
// set file system globals
void load_fs(spb_t spb, int user_in, inode_t* inodes_arr, int inodes_arr_len, char name[255], int db_num);

// fread and fwrite related
size_t read_with_fread(void* infile, int fsize, int count, FILE* ptr_ipt);
size_t write_with_fwrite(void* infile, int fsize, int count, FILE* ptr_ipt);
void* get_one_data_block(int data_index, int offset, int length);
int write_one_data_block(int data_index, void* buffer, int offset, int length);
int read_one_data_block(int data_index, char* buffer, int offset, int length);


int format_disk();
int search_file_in_dir(int the_dir, char* filename);
int write_file_by_inode(f_entry_t* the_file_entry, void* buff, int len);
int read_file_by_inode(f_entry_t* the_file_entry, void* out_buff, int len);

int get_inode_in_open_table(int);
int fd_is_valid(int fd);

#endif