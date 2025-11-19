#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Include your FS header here
#include "bvfs.h"

int main(int argc, char* argv[]) 
{
    	printf("=== BVFS TEST START ===\n");

    	// 1. Attach filesystem (creates and formats if not existing)
    	printf("[TEST] Attaching filesystem...\n");
    	if (bvfs_attach((char*)"testFS.bvfs") != 0) 
	{
		printf("ERROR: Could not attach FS.\n");
        	return 1;
    	}
    	printf("[OK] FS attached.\n");
}

