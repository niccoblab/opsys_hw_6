#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdin.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

//every global in bytes
int BLOCK_SIZE = 512;
//long FILE_SYSTEM_SIZE = 8388608; //I dont think we'll need this. but ill keep it just in case. 
int FILENAME_SIZE = 32; //31 bytes + null byte
int MAX_FILE_SIZE = pow(2, 16); // 2^16 bytes

//other globals needed

int MAX_NUM_OF_FILES = 512;
int MAX_NUM_BLOCKS = 16384;
int attached = 0; //later, check if file system file is created already
uint32_t MAGIC_NUMBER = 0x43110;
#define BVFS_RDONLY 0
#define BVFS_WRCONCAT 1 
#define BVFS_WRTRUNC 2

typedef struct Superblock //should be, like, 16 bytes and contain the location of the first free block list head
{ //this is probably supposed to be smaller, but it makes more sense to me to actually fill it out with stuff & make it the same size as everything else. 
	int totalBlocks; 	//total num of blocks that will be going into the file system
	int numInodes; 		//initialize this early, should have 512 inodes since one inode per file.
	uint16_t firstInode;	//pointer to first Inode
	uint16_t freeListHead;	//pointer to first Free Block Pointer
	uint32_t magicNum;	//for future testing, could return whether the file has been created or not. 
};

typedef struct Inode //128 pointers to diskmap blocks, should be 512 bytes Addendum: ONE INODE PER FILE - will end up with a total of 512 inodes. 
{
	uint16_t diskmap[128];  	//If need to go over multiple blocks, this diskmap is to keep the file intact. 
	char fileName[FILENAME_SIZE]; 	//should be stored here, not in data block
	int fileSize;	   		//self explanatory
	time_t timestamp; 		//timestamp of creation / last edit
	char isFree = 0;		//if the inode contains a file or not. 0 is false.
};

typedef struct Datablock // should also be 512 bytes, contains file data, not files.
{
	char data[BLOCK_SIZE]; //pure, 512 byte array to store file data in. Nothing else needed?
};

typedef struct FreeBlockPointer // should be around 512 bytes, so it fits in one block. 
{
	uint16_t freeBlocks[255];
	uint16_t nextFreeBlockPointer;
}; 

typedef struct FS_STATE //used for saving useful info about the file system 
{
	char *name; //filename
	FILE *disk;			
	Superblock sb;
	Inode *inodeList; //list of inodes
	FreeBlockPointer *freeBlockList; //list of free blocks.

};

FS_STATE state; //global FS_STATE object to keep track of file system information

//idea is, upon attach, to fill the file system size up with free blocks. Start with filling the superblock and inode, and then one block with two FreeBlockPointers,
//pointing to 512 free blocks represented by the Datablock struct. Essentially, one big linked list. 

//write(fd, &fbn, sizeof(fbn));
int bvfs_attach(char *fileSystemName) //Alex will work on this. 
{
	char *filename = fileSystemName;
	//make it so only one file system can be created at a time. 
	if (attached == 1 && strcmp(filename, state.name) == 0)
	{
		return -1;
	}

	//attempt to create file
	int bvFD = open(filename, O_RDWR | O_CREAT | O_EXCL, 0644);	
	if (bvFD < 0) //file already exists, open it and read data.
	{
		if (errno == EEXIST) //errno returns that the file exists.
		{
			FILE *vfs = fopen(filename, "rb+"); //open the file in binary to be read from.
			if (!vfs) //did it open the file?
			{
				return -1; //no, it didnt. File couldn't be opened. 
			}
			state.disk = vfs; //set disk to the file. 

			fseek(vfs, 0, SEEK_SET); //time to get the superblock. 
			size_t getSuperblock = fread(&state.sb, sizeof(Superblock), 1, vfs); //attempt to read from the superblock. 
			
			if (getSuperblock != 1) //did it find the superblock?
			{
				return -1; //no, it didnt.
			}

			if (state.sb.magicNum != MAGIC_NUMBER) //Is this the correct file? 	
			{
				return -1; // it is not.
			}
			
			//now, to fill out the rest of the FS_STATE struct. 
			//start with inodes.
			state.inodeList = malloc(sizeof(Inode) * state.sb.numInodes);
			fseek(vfs, state.sb.firstInode, SEEK_SET);
			fread(state.inodeList, sizeof(Inode), state.sb.numInodes, vfs);
			
			//next, free block list.
			int numFreeBlockPointers = (state.sb.totalBlocks + 255 - 1) / 255;
			state.freeBlockList = malloc(sizeof(FreeBlockPointer) * numFreeBlockPointers);
			fseek(vfs, state.sb.freeListHead * BLOCK_SIZE, SEEK_SET);
			fread(state.freeBlockList, sizeof(FreeBlockPointer), state.sb.totalBlocks, vfs);

			//lastly, the filename because I definitely didnt forget to do it until now.
			state.name = filename;
		}
		else //something happened. Return error.
		{	
			return errno;
		}
	}
	else
	{
		//this is where we create the file if it doesn't already exist.
	}
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

