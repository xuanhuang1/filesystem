#ifndef FS_H
#define FS_H

#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define EMPTY_ENTRY  -1

#include <sys/stat.h>

enum _utype{SUPERUSER, USER, MAXUSER};   
enum {SUCCESS, FAIL};
enum {TRUE = 1, FALSE = 0};
enum {R, W, X};
enum {OPEN_R, OPEN_W, OPEN_RW, OPEN_APPEND};
enum {SEEK_BEGIN=-18, SEEK_CURR=-8,SEEK_ENDFILE=10};




//superblock
typedef struct{
    int type; 				// to specify the type of file system 
	int size; 				/* size of blocks in bytes */
	int inode_offset;		/* inodes offset in blocks */
	int data_offset; 		/* data region offset in blocks */
	int free_inode; 		/* head of free inode list, index */
	int free_block; 		/* head of free block list, index */
}spb_t;

//inode
typedef struct{
	int parent;
	mode_t permission; 		// r w x, user group other (3x3 table in total)
	int isdir;    			// file or directory
	int nextfile;    		// directory ptr for f_readdir, the index of the next file
	int children_num; 		// the number of children for the directory
	int next_free_inode; 	/* index of next free inode */
	int protect; 			/* protection field */
	int nlink; 				/* number of links to this file */
	int size; 				/* numer of bytes in file */
	int uid; 				/* owner’s user ID */
	int gid; 				/* owner’s group ID */
	int ctime; 				/* change time */
	int mtime; 				/* modification time */
	int atime; 				/* access time */
	int dblocks[N_DBLOCKS]; /* pointers to data blocks */
	int iblocks[N_IBLOCKS]; /* pointers to indirect blocks */
	int i2block; 			/* pointer to doubly indirect block */
	int i3block; 			/* pointer to triply indirect block */
}inode_t;

//file entry
typedef struct{
    int mode;      // read write exe
    int offset;    // current ptr offset
    int ind;// ptr to the inode
}f_entry_t; 


//file entry table
typedef struct{
    int length;        		// the length of the open file array, equal to the inode list length
    f_entry_t *open_files;    // array of open file entries, allocated when fs_init()
}f_entry_table_t;
/*
directory
directory is also represented by a inode, 
with its data region storing a list of children 
files. Within its data region, it stores the
list of children’s inode indices followed each 
by their names[len=28]. When a file is deleted
move the last inode index to the current position,
decrement children_num for the directory inode
*/
typedef struct{
	int ind;
	char name[28];
}dir_entry_t;

//stat
typedef struct{
    int mode;
	int nlink; 				/* number of links to this file */
	int size; 				/* numer of bytes in file */
	int uid; 				/* owner’s user ID */
	int gid; 				/* owner’s group ID */
	int ctime; 				/* change time */
	int mtime; 				/* modification time */
	int atime; 				/* access time */
}fs_stat_t;

//filesystem globals, packed into a struct
typedef struct fs_attr_t fs_attr_t;

struct fs_attr_t{
	spb_t spb;
	int data_block_num;
	int u_gid[MAXUSER];    	// stores the group ID, the index is the user ID
	int user;         		// current user ID
	inode_t *shell_d;
	inode_t *root;
	int freeiHead;
	f_entry_table_t table;  // table of opened files, initialized to have only the root and -1s
	char* dataBeginPos;     // ptr to the start of data region
	inode_t* inodes;        // array of inodes
	char diskname[255];     // the disk name
	int free_block_head;
	int errno;
	int fs_num;        		// the file system number
};

#endif