// CSC360 Assignment 3 - Part 4
// Author: Adam Page
// Date: August 1: 2020
//
// FAT-like File system
// This program stores files, passed as an arguments, in the file system
// Arguments: [file system] [file to be stored]
//   file system - the ".img" file system file to be read
//   file to be stored - the name of the file to be stored on the file system
//


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "Constants.h"
#include <string.h>
#include <time.h>

int FAT_search(int FAT[], int FAT_size);
void FAT_load(FILE * input,int FAT[],int FAT_size,int FAT_Start);
void int_2_char(char * arr,int n,int num);

int main(int argc, char **argv){
	if (argc != 3)	{
		fprintf(stderr,"Error please provide two paramenters. A disk image file followed by a file to get from the disk image\n");
		exit(0);
	}

	FILE * input;
	FILE * output;

	if ( (input = fopen(argv[2],"r")) == NULL){
		fprintf(stderr,"Error opening input file: %s\n",argv[2]);
		exit(0);
	}

	if ( (output = fopen(argv[1],"r+") ) == NULL) {
		fprintf(stderr,"Error opening disk image: %s\n",argv[1]);
		exit(0);
	}


	//define some varibles we'll need later
	int i;

	//Byte location of Root Dir
	int Root_Dir_Start = 0;
	fseek(output,ROOTDIRSTART_OFFSET,SEEK_SET);
	int temp[ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET];
	for (i=0;i<ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET;i++){
		temp[i] = fgetc(output);
		Root_Dir_Start += temp[i] << (8*(ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET-i-1));
	}
	Root_Dir_Start *= DEFAULT_BLOCK_SIZE;

	//Byte location of first FAT entry
 	int FAT_Start = 0;	
 	fseek(output,FATSTART_OFFSET,SEEK_SET);
 	for (i=0;i<FATBLOCKS_OFFSET-FATSTART_OFFSET;i++){
 		temp[i] = fgetc(output);
 		FAT_Start += temp[i] << (8*(FATBLOCKS_OFFSET-FATSTART_OFFSET-i-1));
 	}
 	FAT_Start *= DEFAULT_BLOCK_SIZE;

 	//FAT size in number of entries
 	int FAT_size;
	fseek(output,FATBLOCKS_OFFSET,SEEK_SET);
	FAT_size = ((fgetc(output) << 6*4) +
		        (fgetc(output) << 4*4) +
		        (fgetc(output) << 2*4) +
		         fgetc(output)) * FAT_ENTRY_PER_BLOCK;

	//process name of txt file to remove folder address 
	char * filename;
	if ((filename = (strrchr(argv[2],'/'))) == NULL){
		filename = argv[2];
	}
	else{
		filename += 1;
	}
	//ensure length is met
	if (strlen(filename) > DIRECTORY_MAX_NAME_LENGTH){
		fprintf(stderr,"Error: file name exceeds disk limit\n");
		exit(0);
	}
		
	//Load the FAT into an array for processing
	int FAT[FAT_size];
	FAT_load(output,FAT,FAT_size,FAT_Start);

	//SEARCH ROOT DIRECTORY FOR FREE SPOT
	//read root dir into an array for processing
	char bytes[DEFAULT_BLOCK_SIZE];
	fseek(output,Root_Dir_Start,SEEK_SET); 
	for (i=0 ; i < DEFAULT_BLOCK_SIZE ; i++){
		bytes[i] = fgetc(output);
	}		
	
	char digits[4];
	int first_block,mask,flag = 0;
	for(i = 0 ; i < DEFAULT_BLOCK_SIZE ; i += DIRECTORY_ENTRY_SIZE){

		//check directory flag for free spot
		if (bytes[i] == DIRECTORY_ENTRY_FREE){ //We've found a free spot, add to this spot

			flag = 1;

			//Write status byte to root dir on disk (1 byte)
			fseek(output,i+DIRECTORY_STATUS_OFFSET+Root_Dir_Start,SEEK_SET);
			fprintf(output,"%c",DIRECTORY_ENTRY_FILE);


			//Search for first free block in FAT. Write that address to root dir
			first_block = FAT_search(FAT,FAT_size);

			digits[0] = first_block >> 8*3;
			digits[1] = first_block >> 8*2;
			digits[2] = first_block >> 8*1;
			digits[3] = first_block;

			fseek(output,i+DIRECTORY_START_BLOCK_OFFSET+Root_Dir_Start,SEEK_SET);
			fwrite(digits,sizeof(char),sizeof(digits),output);



			//Write File Size (in bytes) (4 bytes)
			int size;
			fseek(input,0,SEEK_END);
			size = ftell(input);

			digits[0] = size >> 8*3;
			digits[1] = size >> 8*2;
			digits[2] = size >> 8*1;
			digits[3] = size;
	
			fseek(output,i+DIRECTORY_FILE_SIZE_OFFSET+Root_Dir_Start,SEEK_SET);	
			fwrite(digits,sizeof(char),sizeof(digits),output);

			//Write Create Time (7 bytes) 
			fseek(output,i+DIRECTORY_CREATE_OFFSET+Root_Dir_Start,SEEK_SET);
			time_t timer;
			struct tm *info;
			time(&timer);

			info = localtime(&timer);
			int year = info->tm_year+1900;
			int month = info->tm_mon+1;
			int day = info->tm_mday;
			int hour = info->tm_hour;
			int min = info->tm_min;
			int sec = info->tm_sec;

			char num[7];
			mask = 0xFF00;

			num[0] = (year&mask)>>8;
			mask = mask >> 8;
			num[1] = (year&mask);
			num[2] = month;
			num[3] = day;
			num[4] = hour;
			num[5] = min;
			num[6] = sec;

			fwrite(num,sizeof(char),sizeof(num),output);	


			//Write Modify Time (7 bytes) (same as create)
			fseek(output,i+DIRECTORY_MODIFY_OFFSET+Root_Dir_Start,SEEK_SET);
			fwrite(num,sizeof(char),sizeof(num),output);


			//Write file name (30 bytes)
			fseek(output,i+DIRECTORY_FILENAME_OFFSET+Root_Dir_Start,SEEK_SET);
			fwrite(filename,sizeof(char),strlen(filename),output);	


			//Write remaining 6 bytes to 0xff
			char ending[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
			fseek(output,i+DIRECTORY_FILENAME_OFFSET+DIRECTORY_MAX_NAME_LENGTH+Root_Dir_Start,SEEK_SET);
			fwrite(ending,sizeof(char),sizeof(ending),output);

			//Loop writing block of DEFAULT_BLOCK_SIZE to free blocks specified by the FAT until all blocks have been written
			int bytes_left = size;
			int block_count = 0;
			int next_block; //first_block will be used for the current block in the loop
			fseek(input,0,SEEK_SET);
			while (bytes_left > 0){
				
				//read first block of bytes from input file
				fread(bytes,sizeof(char),DEFAULT_BLOCK_SIZE,input);

				//set output file pointer to next free block
				fseek(output,DEFAULT_BLOCK_SIZE*first_block,SEEK_SET);
				//write to output file
				fwrite(bytes,sizeof(char),DEFAULT_BLOCK_SIZE,output);

				//update counters
				block_count++;
				bytes_left -= DEFAULT_BLOCK_SIZE;

				if (bytes_left <= 0){
					FAT[first_block] = FAT_EOF;
				}else{
					//search for next free block in fat
					next_block = FAT_search(FAT,FAT_size);
					FAT[first_block] = next_block;
					first_block = next_block;
				}
			}

			//write block count to disk
			fseek(output,i+DIRECTORY_BLOCK_COUNT_OFFSET+Root_Dir_Start,SEEK_SET);
			//convert block_count to an array of chars and write to disk
			digits[0] = block_count >> (8*3);
			digits[1] = block_count >> (8*2);
			digits[2] = block_count >> (8*1);
			digits[3] = block_count;
			fwrite(digits,sizeof(char),sizeof(digits),output);

			//Overwrite old FAT with new FAT
			fseek(output,FAT_Start,SEEK_SET); //FAT start

			for (i=0;i<FAT_size;i++){
				//for each FAtentry, convert to 4 chars and fwrite to disk
				digits[0] = FAT[i] >> (8*3);
				digits[1] = FAT[i] >> (8*2);
				digits[2] = FAT[i] >> (8*1);
				digits[3] = FAT[i];
				fwrite(digits,sizeof(char),sizeof(digits),output);
			}

			break;
		}

	}
	
	if (!flag){
		fprintf(stderr,"Error not free space in root directory on disk: %s\n",argv[1]);
		exit(0);
	}

	if (fclose(input) != 0 || fclose(output) != 0){
		fprintf(stderr,"Error closing files\n");
		exit(0);
	}

	return 0;
}

int FAT_search(int FAT[],int FAT_size){
	int k = 0;
	while(FAT[k] != FAT_FREE){
		k++;
	}
	return k;
}

//function to read the FAT table from the disk img
void FAT_load(FILE * input,int FAT[],int FAT_size,int FAT_Start){
	
	//get the start of the FAT
	
	//set input to the beginning of the FAT
	fseek(input,FAT_Start,SEEK_SET);
	
	//read 4 bytes as one entry to the FAT
	int i;
	for (i=0;i<FAT_size;i++){
		FAT[i] = (fgetc(input) << 6*4) +
				 (fgetc(input) << 4*4) +
				 (fgetc(input) << 2*4) +
				  fgetc(input);
	}
}

