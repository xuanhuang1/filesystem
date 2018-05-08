#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H
#include "../fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int test_init_mount(char* filename);
int f_open(int dir_index, char *filename, int flags);
int f_write(int fd, void *buf, int len);
int f_read(int fd, void *buf, int len);
int f_seek(int fd, int offset, int whence);
int f_rewind(int fd);
int f_close(int fd);
int f_stat(int fd, fs_stat_t *buf);
int f_remove(int dir_index, char* filename);
int f_opendir(int dir_index, char* filename);
int f_readdir(int fd, dir_entry_t *dir_read);
int f_closedir(int fd);
int f_mkdir(int dir_index, char* filename);
int f_rmdir(int dir_index, char* filename);
fs_attr_t* f_mount(char *dir, int flags);
fs_attr_t* f_unmount(char *dir, int flags);


void prt_spb(spb_t spb);
void prt_root(inode_t root);
void prt_inode(inode_t);
void prt_data_region();
void prt_fs();
void prt_table();
void prt_dir_data();
void prt_file_data(int inode_indx);

void set_spb(spb_t* spb, int i_offset, int d_offset, int block_size,int,int);
void set_root(inode_t* root);
void set_empty_inode(inode_t* empty_inode);

// return the number of inodes indicated in spb
int get_inode_count(spb_t spb);
int get_file_size_with_fseek(FILE *ptr_ipt, char** infile);
int extract_next_free_block();
int extract_next_free_inode();
int free_this_inode(int inode_index);
int free_this_block(int data_b_index);

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
int add_one_entry_in_dir(int fd_for_cur_dir, dir_entry_t *dir_entry);
int remove_last_entry_in_dir(int dir_index, dir_entry_t *ret_entry);

int find_replace_dir_entry(int dir_index, char* filename, dir_entry_t *entry_to_write);
int trunc_file(inode_t *inode);
int delete_file_by_inode(int inode_index);

int f_opendir(int dir_index, char* filename);
#endif