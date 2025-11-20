#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bvfs.h"

int main(int argc, char* argv[]) 
{
    	printf("=== BVFS TEST START ===\n");
		
	printf("[TEST] Opening a file from a fs that doesnt exist...\n");
	if(bvfs_open("testfile.txt", BVFS_WRCONCAT) > -1)
	{
		printf("ERROR: Error somewhere. Should be returning -1.");
		return 1;
	}
	printf("[OK] testfile.txt not opened.");
    	
    	printf("[TEST] Attaching filesystem...\n");
    	if (bvfs_attach((char*)"testFS.bvfs") != 0) 
	{
		printf("ERROR: Could not attach FS.\n");
        	return 1;
    	}
    	printf("[OK] FS attached.\n");

	printf("[TEST] Opening a file that doesnt exist in read only...\n");
	if(bvfs_open("testfile.txt", BVFS_RDONLY) > -1)
	{
		printf("ERROR: Error somewhere. Should be returning -1.");
		return 1;
	}
    	printf("[OK] testfile.txt not opened.\n");
	
	printf("[TEST] Opening a file in WRTRUNC so it actually is created...\n");
	if(bvfs_open("testfile.txt", BVFS_WRTRUNC) < 0)
	{
		printf("ERROR: Error somewhere. Probably because you cant read from a file that doesnt exist.");
		return 1;
	}
	printf("[OK] testfile.txt opened.");
}

