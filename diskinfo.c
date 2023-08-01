// CSC360 Assignment 3 - Part 1
// Author: Adam Page
// Date: August 1: 2020
//
// FAT-like File system
// This program will display the information about the supplied file system ie "disc.img".
// It reads the file system super block to display in the FAT
// Arguments: [file system]
//   file system: a ".img" file system file


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Constants.h"



void exersize_print(char * filename);


int main(int argc,char *argv[]){

	if (argc != 2)	{
		fprintf(stderr,"Error please provide one paramenter. A disk image file\n");
		exit(0);
	}

	FILE *fp;

	if ( (fp = fopen(argv[1],"r") ) == NULL) {
		fprintf(stderr,"Error opening file: %s\n",argv[1]);
		exit(0);
	}

	//Read superblock into array for processing
	int block[DEFAULT_BLOCK_SIZE]; 
	int i;
	for (i=0 ; i < DEFAULT_BLOCK_SIZE ; i++){
		block[i] = (int)fgetc(fp);
	}

	i=BLOCKSIZE_OFFSET;

	printf("Super block information:\n");

	//print block size
	printf("Block size: %d\n",(int)(block[i++]*pow(16,2)+block[i++]));

	//print block count
	printf("Block count: %d\n",(int)(block[i++]*pow(16,6) + block[i++]*pow(16,4) + block[i++]*pow(16,2) + block[i++]));

	//print FAT starting block
	printf("FAT starts: %d\n",(int)(block[i++]*pow(16,6) + block[i++]*pow(16,4) + block[i++]*pow(16,2) + block[i++]));

	//print FAT blocks
	int FATblocks = (int)(block[i++]*pow(16,6) + block[i++]*pow(16,4) + block[i++]*pow(16,2) + block[i++]);
	printf("FAT blocks: %d\n",FATblocks);

	//print root directory start
	printf("Root directory start: %d\n",(int)(block[i++]*pow(16,6) + block[i++]*pow(16,4) + block[i++]*pow(16,2) + block[i++]));

	//print root directory blocks
	printf("Root directory blocks: %d\n",(int)(block[i++]*pow(16,6) + block[i++]*pow(16,4) + block[i++]*pow(16,2) + block[i++]));


	int block_count;
	int freeblocks = 0,reservedblocks = 0,allocatedblocks = 0;
	for(block_count = 0 ; block_count < FATblocks ; block_count++){
		for(i=0 ; i < DEFAULT_BLOCK_SIZE/FAT_ENTRY_SIZE ; i++){
			block[i] = (int)(fgetc(fp)*pow(16,6) +
							 fgetc(fp)*pow(16,4) + 
							 fgetc(fp)*pow(16,2) + 
							 fgetc(fp));
		}
		//program gets here, ready to process this block
		for(i=0 ; i < DEFAULT_BLOCK_SIZE/4 ; i++){
			if (block[i] == FAT_FREE){
				freeblocks++;
			}
			else if (block[i] == FAT_RESERVED){
				reservedblocks++;
			}
			else{
				allocatedblocks++;
			}
		}
	}

	printf("\nFAT information:\n");

	//print free blocks
	printf("Free Blocks: %d\n",freeblocks);

	//print number of reserved blocks
	printf("Reserved Blocks: %d\n",reservedblocks);

	//print number of allocated blocks
	printf("Allocated Blocks: %d\n",allocatedblocks);

	if (fclose(fp) != 0){
		fprintf(stderr,"Error closing input file\n");
	}

	return 0;
}
