#include "fs.h"
#include "fs_helper_funcs/helper_funcs.h"

#include "fsCmds.h"

extern fs_attr_t fs;




char* fill_buffer_from_FILE(char* filename, int* size){
	FILE *f = fopen(filename,"r+");
	char *infile_buff = NULL;
	*size = get_file_size_with_fseek(f, &infile_buff);
	read_with_fread(infile_buff, sizeof(char), *size, f);
	printf("file:%s origianl sz %d\n", filename, *size);
	fclose(f);
	return infile_buff;
}


void create_file(char* filename, char* inputname, int dir_index){
	int fd = f_open(dir_index, filename,OPEN_W);
	int sz = 0;
	char* infile_buff = fill_buffer_from_FILE(inputname, &sz);
	int bytes_written = f_write(fd, infile_buff, sz);
	free(infile_buff);
	f_close(fd);
	printf("\nEnd of f_write. bytes_written:%d\n", bytes_written);
}

void append_file(char* filename, char* inputname, int dir_index){
	int fd = f_open(dir_index, filename,OPEN_APPEND);
	int sz = 0;
	char* infile_buff = fill_buffer_from_FILE(inputname, &sz);
	int bytes_written = f_write(fd, infile_buff, sz);
	free(infile_buff);
	f_close(fd);
	printf("\nEnd of f_write. bytes_written:%d\n", bytes_written);
}

void test_create_and_append(){
	//dir_entry_t *dir = (dir_entry_t*)get_one_data_block(0, 0, sizeof(dir_entry_t));
	//printf("dir_entry %s %d\n", dir->name, dir->ind);
	create_file("file1", "test_data_file/data1.txt", 0);
	printf("\n\n\n\n END OF CREATE \n\n\n\n");

	append_file("file1", "test_data_file/append2.txt", 0);
	printf("\n\n\n\n END OF APPEND \n\n\n\n");


	//prt_file_data(fs.table.open_files[fd].ind);


}

void test_read_twice(int fd){
	char* readed_data = (char*)malloc(514);
	char* readed_data2 = (char*)malloc(5);

	readed_data[513] = '\0';
	readed_data2[4] = '\0';

	f_rewind(1);
	int bytes_read = f_read(1, readed_data, 513);
	printf("\nEnd of f_read. bytes_read:%d\n", bytes_read);

	int bytes_read2 = f_read(1, readed_data2, 4);
	printf("\nEnd of f_read. bytes_read2:%d\n", bytes_read2);

	printf("\n\n\n\n END OF READ \n\n\n\n");
	printf("read buffer:\n");
	printf("%s + %s\n", readed_data, readed_data2);
	free(readed_data);
	free(readed_data2);

}

void test_fopen_lv2_dir_newfile(){
	printf("\n\n\nTEST: open 2 files under dir user(1) and rm file1 \n\n\n");

	f_mkdir(0, "user");

	int fd_dir = f_opendir(0, "user");
	int fd = f_open(1, "file1", OPEN_RW);
	int fd2 = f_open(1, "file2", OPEN_RW);

	prt_fs();
	prt_dir_data(1);
	f_close(fd);
	f_remove(1, "file1");
	prt_dir_data(1);
	prt_fs();

}

void test_fopen_2_dirs(){
	printf("\n\n\nTEST: open 2 dir user and spuser under root(0)\n\n\n");

	f_mkdir(0, "user");

	printf("\n\n\n\n END f_mkdir user \n\n\n\n");
	f_mkdir(0, "spuser");
	printf("\n\n\n\n END f_mkdir spuser \n\n\n\n");

	f_opendir(0, "spuser");
	printf("\n\n\n\n END f_opendir spuser \n\n\n\n");

	prt_dir_data(0);
	prt_fs();

}


void test_read_3_files_under_dir(){
	printf("\n\n\nTEST: reading 3 files under user(ind=1)\n\n\n");
	f_mkdir(0, "user");
	int fd = f_opendir(0, "user");
	create_file("file1", "test_data_file/append.txt", 1);
	create_file("file2", "test_data_file/append.txt", 1);
	create_file("file3", "test_data_file/append.txt", 1);
	prt_dir_data(1);
	dir_entry_t dir;

	f_rewind(1);
	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

}


void test_rmdir_with_3_files(){
	printf("\n\n\nTEST: remove user(ind=1) and its 3 children\n\n\n");
	f_mkdir(0, "user");
	int fd = f_opendir(0, "user");
	create_file("file1", "test_data_file/append.txt", 1);
	create_file("file2", "test_data_file/append.txt", 1);
	create_file("file3", "test_data_file/append.txt", 1);
	f_mkdir(1, "empty");

	int fd2 = f_opendir(1, "empty");
	create_file("file4", "test_data_file/append.txt", 5);
	f_closedir(fd2);


	prt_dir_data(1);
	dir_entry_t dir;

	f_rewind(fd);
	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");
	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");
	printf("\n\n\nRM DIR\n\n\n");
	prt_fs();

	f_rmdir(0, "user");
	prt_fs();

}

void test_search_name(){
	printf("\n\n\nTEST: find the name /root/user/file1\n\n\n");
	f_mkdir(0, "user");
	int fd = f_opendir(0, "user");
	create_file("file1", "test_data_file/append.txt", 1);

	printf("Start searching for name\n");
	char* name = search_name_in_dir(1,2);
	printf("the name found:%s\n", name);
	free(name);
}

//////////////// fsCmds test /////////////////
void test_ls(){
	printf("\n\n\nTEST: ls /root : file1 file2 file3\n\n\n");
	create_file("file1", "test_data_file/append.txt", fs.root);
	create_file("file2", "test_data_file/append.txt", fs.root);
	create_file("file3", "test_data_file/append.txt", fs.root);
	shell_ls();
	//shell_cd("/lv1/lv2");
}

void test_pwd(){
	printf("\n\n\nTEST: prt path at /root/lv1/lv2\n\n\n");
	f_mkdir(0, "lv1");
	int fd = f_opendir(0, "lv1");
	f_mkdir(1, "lv2");
	int fd2 = f_opendir(1, "lv2");
	fs.shell_d = 2;
	shell_pwd();
	//shell_cd("/lv1/lv2");
}


void test_cd(){
	printf("\n\n\nTEST: cd to /root/lv1/lv2\n\n\n");
	f_mkdir(0, "lv1");
	int fd = f_opendir(0, "lv1");
	f_mkdir(1, "lv2");
	int fd2 = f_opendir(1, "lv2");
	//fs.shell_d = 2;
	shell_cd("/lv1/lv2");

	shell_pwd();
}

void test_shell_mkdir(){
	printf("\n\n\nTEST: make dir \"dir\" under /root and cd in it\n\n\n");
	shell_mkdir("/lv1");
	shell_cd("./lv1");
	shell_pwd();

	//fs.shell_d = 2;

}

void test_shell_lv2_mkdir(){
	printf("\n\n\nTEST: make dir \"dir\" under /root then \"dir2\" under it\n\n\n");
	shell_mkdir("/dir1");
	//f_opendir(0, "dir1");
	shell_mkdir("/dir1/dir2");

	shell_cd("./dir1/dir2");
	shell_pwd();

	//fs.shell_d = 2;

}

void test_shell_rmdir_with_3_files(){
	printf("\n\n\nTEST: remove /root/user(ind=1) and its 3 children\n\n\n");
	f_mkdir(0, "user");
	int fd = f_opendir(0, "user");
	create_file("file1", "test_data_file/append.txt", 1);
	create_file("file2", "test_data_file/append.txt", 1);
	create_file("file3", "test_data_file/append.txt", 1);
	f_mkdir(1, "empty");

	int fd2 = f_opendir(1, "empty");
	create_file("file4", "test_data_file/append.txt", 5);
	f_closedir(fd2);


	prt_dir_data(1);
	dir_entry_t dir;

	f_rewind(fd);
	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");

	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");
	if(f_readdir(fd, &dir) == SUCCESS)
		printf("dir entry: %d \"%s\"\n", dir.ind, dir.name);
	else printf("Error read dir\n");
	prt_fs();
	
	printf("\n\n\nRM DIR\n\n\n");

	shell_rmdir("/user");
	prt_fs();

}

int main(){
	// one spb, one blk (4) inodes, 2 data blocks (one for root)
	if(shell_mount("disk") == FAIL){
		format_disk();
		shell_mount("disk");
	}

	//test_init_mount("disk");
	prt_fs();
	prt_table();

	printf("\n\n\n\n END OF INIT \n\n\n\n");
	//printf("d entry size:%d, num:%d\n", sizeof(dir_entry_t), fs.spb.size/sizeof(dir_entry_t));

	test_shell_rmdir_with_3_files();

	//prt_data_region();


	//int next_inode = extract_next_free_inode();
	//create_file_at_inode(next_inode, "test_data_file/data1.txt");
	//int fd = f_open(0, "file1",OPEN_APPEND);
	//test_fopen_lv2_dir_newfile();
	//test_read_twice(fd);
	//f_remove(0, "file1");
	//prt_fs();
	//prt_data_region();

	//test_read_twice(fd);
	//test_fopen_lv2_dir_newfile();
	//f_close(fd);
	
	/*
	int fd2 = f_open(0, "file1",OPEN_R);
	test_read_twice(fd2);
	f_close(fd2);*/
	//f_mkdir(0, "dir1");
	//test_fopen_invalid();	
	//prt_table();

	//free(fs.inodes);
	//free(fs.table.open_files);
	shell_unmount(0);
} 

//// HELPER FUNCS ///// 

