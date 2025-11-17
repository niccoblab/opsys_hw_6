#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdin.h>

//every global in bytes
int BLOCK_SIZE = 512;
//long FILE_SYSTEM_SIZE = 8388608; //I dont think we'll need this. but ill keep it just in case. 
int FILENAME_SIZE = 32; //31 bytes + null byte
int MAX_FILE_SIZE = pow(2, 16); // 2^16 bytes

//other globals needed

int MAX_NUM_OF_FILES = 512;
int MAX_NUM_BLOCKS = 16384;
#define BVFS_RDONLY 0;
#define BVFS_WRCONCAT 1; 
#define BVFS_WRTRUNC 2;

struct Superblock //should be, like, 16 bytes and contain the location of the first free block list head
{
	int totalBlocks = MAX_NUM_BLOCKS; //total num of blocks that will be going into the file system
	uint16_t firstInode;		  //pointer to first Inode
	uint16_t freeListHead;		  //pointer to first Free Block Pointer
	int size = BLOCK_SIZE;		  //size of superblock, same as every other block. 
	int doesExist = 0		  //for future testing, could return whether the file has been created or not. 
};

struct Inode //128 pointers to diskmap blocks, should be 512 bytes Addendum: ONE INODE PER FILE - will end up with a total of 512 inodes. 
{
	uint16_t diskmap[128];  	//If need to go over multiple blocks, this diskmap is to keep the file intact. 
	char fileName[FILENAME_SIZE]; 	//should be stored here, not in data block
	int fileSize;	   		//self explanatory
	time_t timestamp; 		//timestamp of creation / last edit
	char isFree = 0;		//if the inode contains a file or not. 0 is false.
};

struct Datablock // should also be 512 bytes, contains file data, not files.
{
	char data[BLOCK_SIZE]; //pure, 512 byte array to store file data in. Nothing else needed?
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

