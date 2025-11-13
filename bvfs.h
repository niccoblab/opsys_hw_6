#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct Superblock
{

};

struct Inode //128 pointers to diskmap blocks, should be 512 bytes
{
	uint16_t diskmap[128]
	uint8_t padding[212];
};

struct Datablock
{
	
};

struct FreeBlockNode
{
	uint16_t freeBlocks[255];
	uint16_t next;
};

//every global in bytes
int MAX_BLOCK_SIZE = 512
long FILE_SYSTEM_SIZE = 8388608
int FILENAME_SIZE = 33 //32 bytes + null byte
int MAX_FILE_SIZE = (2, 16) // 2^16 bytes


//other globals needed

int MAX_NUM_OF_FILES = 512

//write(fd, &fbn, sizeof(fbn));
int bvfs_attach(char *fileSystemName)
{

}

int bvfs_detach()
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

