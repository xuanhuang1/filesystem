#ifndef FSCMD_H
#define FSCMD_H

int shell_init_fs();

int shell_ls();
int shell_chmod();
int shell_mkdir(char* pathname);
int shell_rmdir(char* pathname);
int shell_cd(char* pathname);
int shell_pwd();
int shell_cat(char* pathname);
int shell_more(char* pathname);
int shell_rm(char* pathname);
int shell_mount(char* pathname);
int shell_unmount(int index);



#endif