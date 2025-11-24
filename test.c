#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bvfs.h"

int main(int argc, char* argv[]) 
{
	const char *filename = "testFS.bvfs";
	remove(filename);

    	printf("=== BVFS TEST START ===\n");
		
	printf("[TEST] Opening a file from a fs that doesnt exist...\n");
	if(bvfs_open("testfile.txt", BVFS_WRCONCAT) > -1)
	{
		printf("ERROR: Error somewhere. Should be returning -1.\n");
		return 1;
	}
	printf("[OK] testfile.txt not opened.\n\n");
    	
    	printf("[TEST] Attaching filesystem...\n");
    	if (bvfs_attach((char*)"testFS.bvfs") != 0) 
	{
		printf("ERROR: Could not attach FS.\n");
        	return 1;
    	}
    	printf("[OK] FS attached.\n\n");

	printf("[TEST] Opening a file that doesnt exist in read only...\n");
	if(bvfs_open("testfile.txt", BVFS_RDONLY) > -1)
	{
		printf("ERROR: Error somewhere. Should be returning -1.\n");
		return 1;
	}
    	printf("[OK] testfile.txt not opened.\n\n");
		
	printf("[TEST] Opening a file in WRTRUNC so it actually is created...\n");
	if(bvfs_open("testfile.txt", BVFS_WRTRUNC) < 0)
	{
		printf("ERROR: Error somewhere \n");
		return 1;
	}
	printf("[OK] testfile.txt opened \n\n");
	
	printf("[TEST] Attempting to detach the file system...\n");
	if(bvfs_detach() < 0)
	{
		printf("ERROR: Error somewhere. \n");
		return 1;
	}
	printf("[OK] testfile.txt detached.\n\n");
	
	printf("[TEST] Attempting to detach the file system again...\n");
	if(bvfs_detach() != -1)
	{
		printf("ERROR: Error somewhere. Somehow tried to detach anyway.\n");
		return 1;
	}
	printf("[OK] testfile.txt still detached.\n\n");

}

