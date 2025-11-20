#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

//every global in bytes
#define BLOCK_SIZE 512 
#define FILENAME_SIZE 32 //31 bytes + null byte
#define MAX_FILE_SIZE (1 << 16) // 2^16 bytes

//other globals needed

#define MAX_NUM_OF_FILES 512
#define MAX_NUM_BLOCKS 16384
#define MAGIC_NUMBER 0x43110U
#define BVFS_RDONLY 0
#define BVFS_WRCONCAT 1 
#define BVFS_WRTRUNC 2

typedef struct Superblock //should be, like, 16 bytes and contain the location of the first free block list head
{ //this is probably supposed to be smaller, but it makes more sense to me to actually fill it out with stuff & make it the same size as everything else. 
	uint32_t magicNum;	 //for future testing, could return whether the file has been created or not. 
	uint32_t totalBlocks; 	 //total num of blocks that will be going into the file system
	uint32_t numInodes; 		 //initialize this early, should have 512 inodes since one inode per file.
	uint16_t firstInode;	 //pointer to first Inode
	uint16_t freeListHead;	 //pointer to first Free Block Pointer
	uint16_t firstDatablock; //pointer to the first data block for easy access later. Will, theoretically, never be changed. 
} Superblock;

typedef struct Inode //128 pointers to diskmap blocks, should be 512 bytes Addendum: ONE INODE PER FILE - will end up with a total of 512 inodes. 
{
	uint16_t diskmap[128];  	//If need to go over multiple blocks, this diskmap is to keep the file intact. 
	char fileName[FILENAME_SIZE]; 	//should be stored here, not in data block
	uint32_t fileSize;	   		//self explanatory
	time_t timestamp; 		//timestamp of creation / last edit
	uint8_t isFree;			//if the inode contains a file or not. 0 is false.i
} Inode;

typedef struct FreeBlockPointer // should be around 512 bytes, so it fits in one block. 
{
	uint16_t freeBlocks[255];
	uint16_t nextFreeBlockPointer;
} FreeBlockPointer; 

typedef struct FS_STATE //used for saving useful info about the file system 
{
	char *name; //filename
	int diskFD;			
	Superblock sb;
	Inode *inodeList; //list of inodes
	FreeBlockPointer *freeBlockList; //list of free blocks.

} FS_STATE;

static FS_STATE state = {0}; //global FS_STATE object to keep track of file system information
static int attached = 0;

//idea is, upon attach, to fill the file system size up with free blocks. Start with filling the superblock and inode, and then one block with two FreeBlockPointers,
//pointing to 512 free blocks represented by the Datablock struct. Essentially, one big linked list. 

//write(fd, &fbn, sizeof(fbn));
int bvfs_attach(char *fileSystemName) //Alex will work on this. 
{
	char *filename = fileSystemName;
	//make it so only one file system can be created at a time. 
	if (attached && state.name && strcmp(filename, state.name) == 0)
	{
		printf("Error: file system already attached or you have another file system already.\n")
		return -1;
	}
	
	//attempt to create file
	int bvFD = open(filename, O_RDWR | O_CREAT | O_EXCL, 0644);	
	if (bvFD < 0) //file already exists, open it and read data.
	{
		if (errno == EEXIST) //errno returns that the file exists.
		{
			bvFD = open(filename, O_RDWR); //open the file to be read from.
			if (bvFD < 0) return -1; //check if it found the file. 
			state.diskFD = bvFD; //set disk to the file. 

			if (lseek(bvFD, 0, SEEK_SET) < 0) //time to get the superblock. Find its location, return -1 if not found. 
			{ 
				printf("Error: Could not find the superblock's location.\n");
				close(bvFD); 
				return -1; 
			}
			ssize_t getSuperblock = read(bvFD, &state.sb, sizeof(Superblock)); //attempt to read from the superblock. 
			
			if (getSuperblock != sizeof(Superblock)) //did it find the superblock?
			{
				printf("Error: Could not get the superblock.\n");
				close(bvFD);
				return -1; //no, it didnt
			}

			if (state.sb.magicNum != MAGIC_NUMBER) //Is this the correct file? 	
			{
				printf("Error: Magic numbers do not match, not the correct file system. ");
				close(bvFD);
				return -1; // it is not.
			}
			
			//now, to fill out the rest of the FS_STATE struct. 
			//start with inodes.
			state.inodeList = malloc(sizeof(Inode) * state.sb.numInodes);
			off_t inodeOffset = (off_t)state.sb.firstInode * BLOCK_SIZE; //offset variable to determine where the inodes start. 
			if (lseek(bvFD, inodeOffset, SEEK_SET) < 0) //...did we find it?
			{
				printf("Error: Could not find where the inodes begin.");
				free(state.inodeList);
				close(bvFD);
				return -1;
			}
			//lets check if we are able to properly read it.
			if (read(bvFD, state.inodeList, sizeof(Inode) * state.sb.numInodes) != (ssize_t)(sizeof(Inode) * state.sb.numInodes))
			{
				printf("Error: Could not read the inode list.");
				free(state.inodeList);
				close(bvFD);
				return -1;
			}
			//end of inode stuff
			//next, free block list. First, we gotta find out how many free block pointers there are. 
			int numFreeBlockPointers = 1;
       			while (1)
			{
				uint32_t usedBeforeData = 1 + state.sb.numInodes + numFreeBlockPointers; //superblock + inodes + current count of free block pointers.
				if (usedBeforeData >= state.sb.totalBlocks) //file system invalid :( 
				{
					printf("Error: File System Invalid. More used blocks than total blocks in file system.");
					free(state.inodeList);
					close(bvFD);
					return -1;
				}
				uint32_t dataBlocks = state.sb.totalBlocks - usedBeforeData;
				uint32_t neededFBP = (dataBlocks + 255 - 1) / 255;
				if (neededFBP == (uint32_t)numFreeBlockPointers) break;
				numFreeBlockPointers = neededFBP;
			}	
			//cool! now we actually make the list.
			state.freeBlockList = malloc(sizeof(FreeBlockPointer) * numFreeBlockPointers);
			if (!state.freeBlockList) 
			{
				printf("Error: Malloc for the free block pointer list failed.");
				free(state.inodeList);
				close(bvFD);
				return -1;
			}
			off_t fbpOffset = (off_t)state.sb.freeListHead * BLOCK_SIZE; //offset variable to determine where the free list stuff starts.
			if (lseek(bvFD, fbpOffset, SEEK_SET) < 0) //...did we find it?
			{
				printf("Error: Could not find the free block list in the system.");
				free(state.freeBlockList);
				free(state.inodeList);
				close(bvFD);
				return -1;
			}
			//lets check if we were able to read the FBPs
			if (read(bvFD, state.freeBlockList, sizeof(FreeBlockPointer) * numFreeBlockPointers) != (ssize_t)(sizeof(FreeBlockPointer) * numFreeBlockPointers))
			{
				printf("Error: Could not read the free block pointers from the disk.");
				free(state.freeBlockList);
				free(state.inodeList);
				close(bvFD);
				return -1;
			}
			
			//lastly, the filename because I definitely didnt forget to do it until now.
			state.name = filename;
			attached = 1;
			return 0;
		}
		else //something happened. Return error.
		{	
			return errno;
		}
		//uuuuuuuuggggggggghhhhhhhhh are we done yet
	}

	//goddammit
	//It didn't find an already existing file, so we'll just create a new one.
	//set file descriptor and filename in state
	state.name = filename;
	state.diskFD = bvFD;

	//fill out Superblock info first and foremost.
	state.sb.totalBlocks = MAX_NUM_BLOCKS; 
	state.sb.numInodes = MAX_NUM_OF_FILES;
	state.sb.firstInode = 1; //first inode will probably go right after the superblock, which is at 0
	uint32_t inodeBlocks = (uint32_t)((sizeof(Inode) * state.sb.numInodes + BLOCK_SIZE - 1) / BLOCK_SIZE); //will help determine where free list head is going.
	uint32_t freeListHead = state.sb.firstInode + inodeBlocks; 
	state.sb.freeListHead = (uint16_t)freeListHead; //head of free linked list will probably go after the last Inode. Convert a 32bit integer to 16 bit. 
	state.sb.magicNum = MAGIC_NUMBER;

	//find Superblock position & write it into the file. 
	if (lseek(bvFD, 0, SEEK_SET) < 0) //can we find the place to put it? no? 
	{
		printf("Error: Could not find superblock position, which is weird because its always at 0.");
		close(bvFD);
		return -1;
	}
	if (write(bvFD, &state.sb, sizeof(Superblock)) != (ssize_t)sizeof(Superblock)) //can we properly write to the file?
	{
		printf("Error: Could not write the superblock to the disk.");
		close(bvFD);
		return -1;
	}

	//write space for inodes in the file & add inodes to the inodeList array, then find the location of the first inode and write the inodes to the file.
	state.inodeList = calloc(state.sb.numInodes, sizeof(Inode)); //make space for the inode list and fill with zeros with calloc because i dont want to fill it in the loop
	if (!state.inodeList) //did
	{		      //did we do it
		printf("Error: Could not make space for the inode list.");
		close(bvFD);  //did we do the thing
		return -1;    //did we create the space for the inode list
	}		      //i hope so. can you tell im trying to entertain myself as I write the same comments over and over?
	for (uint32_t i = 0; i < state.sb.numInodes; i++) //add inodes to inodeList array
	{
		state.inodeList[i].isFree = 1;
		state.inodeList[i].fileSize = 0;
		state.inodeList[i].timestamp = 0;
	}

	off_t inodeOffset = (off_t)state.sb.firstInode * BLOCK_SIZE; //make offset to write the inode list after the superblock.
	if (lseek(bvFD, inodeOffset, SEEK_SET) < 0) //did we find place to put inode list 
	{
		printf("Error: Could not find where to place the inode list.");
		free(state.inodeList); //no? erase all progress so far and crush my dreams
		close(bvFD);
		return -1;
	}

        if (write(bvFD, state.inodeList, sizeof(Inode) * state.sb.numInodes) != (ssize_t)(sizeof(Inode) * state.sb.numInodes)) 
	{				//write time :)
		printf("Error: Could not write the inode list to the disk.");
		free(state.inodeList);	  //how much?
		close(bvFD);		//secret :3
		return -1;		  //why?
					//just guess
					  //*attempts to write to this location*
					//error :(
	}

	//lets... uh... make the free block pointers again.
	//first, though, lets find out how many free block pointers we need 
	int numFreeBlockPointers = 1;	
	while (1) //listen its almost the exact same code as the one above soooooooo
	{
		uint32_t usedBeforeData = 1 + inodeBlocks + numFreeBlockPointers; //superblock + inodes + current count of free block pointers.
		if (usedBeforeData >= state.sb.totalBlocks) //file system invalid :( 
		{
			printf("Error: More used data than total blocks in the system. How did you manage this?");
			free(state.inodeList);
			close(bvFD);
			return -1;
		}
		uint32_t dataBlocks = state.sb.totalBlocks - usedBeforeData; 
		uint32_t neededFBP = (dataBlocks + 255 - 1) / 255;
		if (neededFBP == (uint32_t)numFreeBlockPointers) break;
		numFreeBlockPointers = neededFBP;
	}	
	
	//store the first data block and fill the free pointer list in memory
	state.sb.freeListHead = (uint16_t)(state.sb.firstInode + inodeBlocks);
	state.sb.firstDatablock = (uint16_t)(state.sb.freeListHead + numFreeBlockPointers);

	state.freeBlockList = calloc(numFreeBlockPointers, sizeof(FreeBlockPointer)); //make space for the free block list.
	if (!state.freeBlockList)
	{
		printf("Error: Could not make space for the free block list.");
		free(state.inodeList);
		close(bvFD);
		return -1;
	}
	
	//begin hell. and by hell, i mean linked list creation. 
	uint32_t blockIndex = state.sb.firstDatablock;
	for (int i = 0; i < numFreeBlockPointers; i++) //outer for loop to create all free block pointers potentially needed. 
	{
		FreeBlockPointer *fbp = &state.freeBlockList[i];
		for (int j = 0; j < 255; j++) //inner for loop meant to loop through the free blocks in order to put them in the FreeBlockPointer! Wow!
		{
			if (blockIndex < state.sb.totalBlocks) 
			{
				fbp->freeBlocks[j] = (uint16_t)blockIndex++;
			}
			else
			{
				fbp->freeBlocks[j] = 0xFFFF; //in case there is no free block here 
			}
		}
		if (i == numFreeBlockPointers - 1) //is this the last free block pointer?
		{
			fbp->nextFreeBlockPointer = 0xFFFF; //set the next to nothing
		}
		else
		{
			fbp->nextFreeBlockPointer = (uint16_t)(state.sb.freeListHead + i + 1); //set the next free block pointer to the next one! Woah!
		}
	}

	if (lseek(bvFD, state.sb.freeListHead * BLOCK_SIZE, SEEK_SET) < 0) //try to find the place for the free list head location
	{
		printf("Error: Could not find the place on disk to write the free block lists.");
		free(state.freeBlockList); //do you ever feel like bashing your head into a brick wall?
		free(state.inodeList);	   //thats what writing these repetitive checks do to me.
		close(bvFD);
		return -1;
	}
	if (write(bvFD, state.freeBlockList, sizeof(FreeBlockPointer) * numFreeBlockPointers) != (ssize_t)(sizeof(FreeBlockPointer) * numFreeBlockPointers))
	{				 	//Try to write the free block lists. some slam poetry for you:
		printf("Error: Could not write the free block list to disk.");
		free(state.freeBlockList); 	//do. you ever feel. like a plastic bag.
		free(state.inodeList);	   	//drifing. through the wind.
		close(bvFD);			//wanting to start again. 
		return -1;			//do you. ever feel. like a. house of cards.
	}					//one blow. from caving in.
	
	//a-are we done? Can I go to sleep now?
	//oh wait. we need to fill the file system with stuff. 
	//ugh. fine. 
	off_t fsSize = (off_t)state.sb.totalBlocks * BLOCK_SIZE;
	if (ftruncate(bvFD, fsSize) < 0) //try to fill the rest of the file with null bytes and cut off any excess data.
	{				 
		printf("Error: Could not fill the file system with overwritable data.");
		free(state.freeBlockList); //im going to be seeing this in my dreams for the next month.
		free(state.inodeList);	   //or are they nightmares? i dont know anymore.
		close(bvFD);
		return -1;
	}

	attached = 1;
	return 0;
	//w-what? i-i'm done??? i... i can't believe it... i'm free...
	//oh theres more to code. at the time of writing this, the other seven functions need to be completed. oh god.
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

