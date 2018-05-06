#include "helper_funcs.h"
#include <assert.h>

extern fs_attr_t fs;



// get data block by index
void* get_one_data_block(int data_index, int offset, int length){
	assert (offset+length<fs.spb.size+1);
	FILE *ptr_ipt = fopen("disk","r");
	assert (ptr_ipt != NULL);
	char* buffer = (char*)malloc(length+1);
	buffer[length] = '\0';
	int data_offset_on_disk = fs.spb.size+fs.spb.size*(fs.spb.data_offset+data_index);
	printf("\n***get data_off_on_disk %d+%d len:%d***\n", data_offset_on_disk, offset, length);
	fseek(ptr_ipt, data_offset_on_disk+offset, SEEK_SET);
	read_with_fread(buffer, length, 1, ptr_ipt);
	fclose(ptr_ipt);
	return buffer;
}


// read file with fread and check if success
size_t read_with_fread(void* infile, int fsize, int count, FILE* ptr_ipt){
	size_t newLen = fread(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error reading file\n");
	return newLen*fsize;
}

// get data block by index
int read_one_data_block(int data_index, char* buffer, int offset, int length){
	assert (offset+length<fs.spb.size+1);
	FILE *ptr_ipt = fopen("disk","r");

	assert (ptr_ipt != NULL);
	int data_offset_on_disk = fs.spb.size+fs.spb.size*(fs.spb.data_offset+data_index);
	printf("\n***read data_off_on_disk %d+%d len:%d***\n", data_offset_on_disk, offset, length);
	fseek(ptr_ipt, data_offset_on_disk+offset, SEEK_SET);
	int size_read = read_with_fread(buffer, length, 1, ptr_ipt);
	fclose(ptr_ipt);
	return size_read;
}

size_t write_with_fwrite(void* infile, int fsize, int count, FILE* ptr_ipt){
	printf("\t\twirte to file\n");
	//printf("--%s--\n", infile);
	size_t newLen = fwrite(infile, fsize, count, ptr_ipt);
	if(!newLen) perror("Error writing file");
	return newLen*fsize;
}
// write data block pointed by index
int write_one_data_block(int data_index, void* buffer, int offset, int length){
	assert (offset+length < fs.spb.size+1);
	FILE *ptr_ipt = fopen(fs.diskname,"r+");

	assert (ptr_ipt != NULL);
	int data_offset_on_disk = fs.spb.size+fs.spb.size*(fs.spb.data_offset+data_index);
	printf("\twrite data_off_on_disk %d+%d len=%d. with datablock index: %d\n", data_offset_on_disk, offset, length, data_index);
	fseek(ptr_ipt, data_offset_on_disk+offset, SEEK_SET);
	int size_written = write_with_fwrite(buffer, length, 1,  ptr_ipt);
	fclose(ptr_ipt);

	return size_written;
}

int extract_next_free_block(){
	int ret_index = fs.free_block_head;
	char* the_free_block = get_one_data_block(ret_index, 0, fs.spb.size);
	fs.free_block_head = *((int*)the_free_block);
	if(fs.free_block_head == -1){
		printf("no block left!\n");
		assert (fs.free_block_head != -1);
		return FAIL;
	}
	free(the_free_block);
	printf("\textract free block:%d, set f head:%d\n", ret_index, fs.free_block_head);
	return ret_index;
}
