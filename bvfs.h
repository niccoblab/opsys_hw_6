#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdin.h>

//every global in bytes
int MAX_BLOCK_SIZE = 512
long FILE_SYSTEM_SIZE = 8388608
int FILENAME_SIZE = 32 //31 bytes + null byte
int MAX_FILE_SIZE = pow(2, 16) // 2^16 bytes


//other globals needed

int MAX_NUM_OF_FILES = 512
#define BVFS_RDONLY 0 
#define BVFS_WRCONCAT 1 
#define BVFS_WRTRUNC 2

struct Superblock //should be, like, 16 bytes and contain the location of the first free block list head
{
	uint16_t free_list_head;
};

struct Inode //128 pointers to diskmap blocks, should be 512 bytes
{
	uint16_t diskmap[128];
	uint8_t padding[212];
};

struct Datablock // should also be 512 bytes, contains files
{
	char *filename //should be 32 bytes in total
	char isFree;
};

struct FreeBlockPointer // should be around 256 bytes, so two can fit in one block. 
{
	uint16_t freeBlocks[255];
	uint16_t nextFreeBlockPointer;
}; 

//idea is, upon attach, to fill the file system size up with free blocks. Start with filling the superblock and inode, and then one block with two FreeBlockPointers,
//pointing to 512 free blocks represented by the Datablock struct. Essentially, one big linked list. 

//write(fd, &fbn, sizeof(fbn));
int bvfs_attach(char *fileSystemName) //Alex will work on this. 
{
	
}

int bvfs_detach() //Nicco will work on this.
{

}

int bvfs_open(char *filename, int mode)
{

}

int bvfs_close(int bvfsFD)
{

}

int bvfs_read(int bvfsFD, char *buffer, size_t numBytes)
{

}

int bvfs_write(int bvfdFD, char *buffer, size_t numBytes)
{

}

int bvfs_unlink(char *filename)
{
	
}

void bfvs_ls()
{

}

