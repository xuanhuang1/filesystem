#include "helper_funcs.h"
#include <assert.h>

extern fs_attr_t fs;


// read file with fread and check if success
size_t read_f_into_buffer(char* infile, int fsize, int count, FILE* ptr_ipt){
	size_t newLen = fread(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error reading file\n");
	return newLen;
}

// get data block by index
char* get_data_block(int data_index, int offset, int length){
	assert (offset+length<fs.spb.size+1);
	FILE *ptr_ipt = fopen("disk","r");
	assert (ptr_ipt != NULL);
	char* buffer = (char*)malloc(length);
	int data_offset_on_disk = fs.spb.size+fs.spb.size*(fs.spb.data_offset+data_index)+offset;
	printf("\n***read data_off_on_disk %d+%d len:%d***\n", data_offset_on_disk, offset, length);
	fseek(ptr_ipt, data_offset_on_disk+offset, SEEK_SET);
	read_f_into_buffer(buffer, length, 1, ptr_ipt);
	fclose(ptr_ipt);
	return buffer;
}

size_t write_f_from_buffer(char* infile, int fsize, int count, FILE* ptr_ipt){
	printf("wirte to file\n");
	printf("%s\n", infile);
	size_t newLen = fwrite(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error writing file");
	return newLen;
}
// write data block pointed by index
char* write_data_block(int data_index, char* buffer, int offset, int length){
	assert (offset+length < fs.spb.size+1);
	FILE *ptr_ipt = fopen(fs.diskname,"r+");

	assert (ptr_ipt != NULL);
	int data_offset_on_disk = fs.spb.size+fs.spb.size*(fs.spb.data_offset+data_index)+offset;
	printf("write data_off_on_disk %d. with index: %d\n", data_offset_on_disk+offset, data_index);
	fseek(ptr_ipt, data_offset_on_disk+offset, SEEK_SET);
	write_f_from_buffer(buffer, length, 1,  ptr_ipt);
	fclose(ptr_ipt);

	return buffer;
}



int table_entry_valid(int i){
	if(fs.table.open_files[i].ind == EMPTY_ENTRY){ 
		printf("table[%d] invalid!\n", i);
		return FALSE;
	}
	return TRUE;
}


int if_file_permission(inode_t i, int flag){
	int fileMode = i.permission;
	if(flag == R){
		if(i.uid == fs.user) 			return (fileMode & S_IRUSR) && (fileMode & S_IREAD);
		if(i.gid == fs.u_gid[fs.user]) 	return (fileMode & S_IRGRP) && (fileMode & S_IREAD);
		else							return (fileMode & S_IROTH) && (fileMode & S_IREAD);
	}else if(flag == W){
		if(i.uid == fs.user) 			return (fileMode & S_IRUSR) && (fileMode & S_IWRITE);
		if(i.gid == fs.u_gid[fs.user]) 	return (fileMode & S_IRGRP) && (fileMode & S_IWRITE);
		else				 			return (fileMode & S_IROTH) && (fileMode & S_IWRITE);
	}else{
		if(i.uid == fs.user) 			return (fileMode & S_IRUSR) && (fileMode & S_IEXEC);
		if(i.gid == fs.u_gid[fs.user]) 	return (fileMode & S_IRGRP) && (fileMode & S_IEXEC);
		else				 			return (fileMode & S_IROTH) && (fileMode & S_IEXEC);
	}
}
