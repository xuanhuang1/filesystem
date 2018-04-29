#include "helper_funcs.h"


extern fs_attr_t fs;


// read file with fread and check if success
size_t read_f_into_buffer(char* infile, int fsize, int count, FILE* ptr_ipt){
	size_t newLen = fread(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error reading file\n");
	return newLen;
}

// get data block by index
char* get_data_block(char diskname[255], int data_index, spb_t spb){
	FILE *ptr_ipt = fopen("disk","r");
	char* buffer = (char*)malloc(sizeof(spb.size));
	int data_offset_on_disk = spb.size+spb.size*(spb.data_offset+data_index);
	printf("data_off_on_disk %d\n", data_offset_on_disk);
	fseek(ptr_ipt, data_offset_on_disk, SEEK_SET);
	read_f_into_buffer(buffer, sizeof(char), sizeof(spb.size), ptr_ipt);
	fclose(ptr_ipt);
	return buffer;
}
