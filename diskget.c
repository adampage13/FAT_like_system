// CSC360 Assignment 3 - Part 3
// Author: Adam Page
// Date: August 1: 2020
//
// FAT-like File system
// This program copies files from the file system to the current UNIX directory
// Arguments: [file system] [file to be retrieved]
//   file system - the ".img" file system file to be read
//   file to be retrieved - the name of the file to be read from the file system
//



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "Constants.h"
#include <string.h>

void print_block(int bytes[]);
void FAT_load(FILE * input,int FAT[],int FAT_size,int FAT_Start);

int main(int argc, char **argv){
	if (argc != 3)	{
		fprintf(stderr,"Error please provide two paramenters. A disk image file followed by a file to get from the disk image\n");
		exit(0);
	}

	FILE * input;
	FILE * output;

	if ( (input = fopen(argv[1],"r") ) == NULL) {
		fprintf(stderr,"Error opening file: %s\n",argv[1]);
	}

	//Define some variables we'll need later

	int block,num_blocks,i;

	//Byte location of first FAT entry
 	int FAT_Start = 0;	
 	int temp_arr[FATBLOCKS_OFFSET-FATSTART_OFFSET];
 	fseek(input,FATSTART_OFFSET,SEEK_SET);
 	for (i=0;i<FATBLOCKS_OFFSET-FATSTART_OFFSET;i++){
 		temp_arr[i] = fgetc(input);
 		FAT_Start += temp_arr[i] << (8*(FATBLOCKS_OFFSET-FATSTART_OFFSET-i-1));
 	}
 	FAT_Start *= DEFAULT_BLOCK_SIZE;

	//get FAT size from super block
	int FAT_size;
	fseek(input,FATBLOCKS_OFFSET,SEEK_SET);
	FAT_size = ((fgetc(input) << 6*4) +
		        (fgetc(input) << 4*4) +
		        (fgetc(input) << 2*4) +
		         fgetc(input)) * FAT_ENTRY_PER_BLOCK;
	


	//Load the FAT into an array for easy processing
	int FAT[FAT_size];
	FAT_load(input,FAT,FAT_size,FAT_Start);

	//Byte location of Root Dir
	int Root_Dir_Start = 0;
	fseek(input,ROOTDIRSTART_OFFSET,SEEK_SET);
	for (i=0;i<ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET;i++){
		temp_arr[i] = fgetc(input);
		Root_Dir_Start += temp_arr[i] << (8*(ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET-i-1));
	}
	Root_Dir_Start *= DEFAULT_BLOCK_SIZE;



	//TESTING --- PRINT FAT
	/*
	fseek(input,FAT_Size,SEEK_SET);

	//print FAT
	printf("PRINTING DIRECTLY READ FAT\n");
	int a,count=0,newline = 0;
	for (a=0 ; a < 512*120 ; a++){
		printf("%02x",(int)fgetc(input));
		count++;
		if (count == 4){
			printf(" ");
			count = 0;
			newline++;
			if (newline == 16){
				printf("\n");
				newline = 0;
			}
		}
	}

	printf("PRINTING ARRAY FAT\n");
	for (a=0 ; a < FAT_size ; a++){
		printf("%x",FAT[a]);;
		count++;
		if (count == 1){
			printf(" ");
			count = 0;
			newline++;
			if (newline == 16){
				printf("\n");
				newline = 0;
			}
		}
	}
	
	*/


	//Read Root Directory into array for processing
	int bytes[DEFAULT_BLOCK_SIZE];
	fseek(input,Root_Dir_Start,SEEK_SET); 
	for (i=0 ; i < DEFAULT_BLOCK_SIZE ; i++){
		bytes[i] = fgetc(input);
	}		

	//search for file name
	char temp[DIRECTORY_MAX_NAME_LENGTH];
	int j,flag = 0;
	for(i = 0 ; i < DEFAULT_BLOCK_SIZE ; i += DIRECTORY_ENTRY_SIZE){
		for (j=0;j<DIRECTORY_MAX_NAME_LENGTH;j++){
			temp[j] = bytes[i+DIRECTORY_FILENAME_OFFSET+j];
		}

		//check if file name matches this directories name
		if (!strcmp(temp,argv[2])){
			flag = 1;
			
			//get starting block
			j = i;
			block = (bytes[DIRECTORY_START_BLOCK_OFFSET + j++] << 6*4) +
					 	  (bytes[DIRECTORY_START_BLOCK_OFFSET + j++] << 4*4) +
					      (bytes[DIRECTORY_START_BLOCK_OFFSET + j++] << 2*4) +
					       bytes[DIRECTORY_START_BLOCK_OFFSET + j++];
	
			//get number of blocks used by file	
			j = i;
			num_blocks =  (bytes[DIRECTORY_BLOCK_COUNT_OFFSET + j++] << 6*4) +
					      (bytes[DIRECTORY_BLOCK_COUNT_OFFSET + j++] << 4*4) +
					      (bytes[DIRECTORY_BLOCK_COUNT_OFFSET + j++] << 2*4) +
					       bytes[DIRECTORY_BLOCK_COUNT_OFFSET + j++];
	

			//while loop .... read data from start block then go to that entry of the FAT table. 
			//Terminate when 0xFFFFFFFF is found in the FAT

			if ((output = fopen(argv[2],"w")) == NULL){
				fprintf(stderr,"Error opening output file\n");
				exit(0);
			}

			int count = 0,k;
			char block_data[DEFAULT_BLOCK_SIZE];
			for (k=0;k<DEFAULT_BLOCK_SIZE;k++){
				block_data[k] = 0;
			}
			do {
				//Read data from block block
				fseek(input,block*DEFAULT_BLOCK_SIZE,SEEK_SET);
				fread(block_data,sizeof(char),DEFAULT_BLOCK_SIZE,input);

				//write to output
				if (block_data[DEFAULT_BLOCK_SIZE-1] != 0){
					fwrite(block_data,sizeof(char),DEFAULT_BLOCK_SIZE,output);
				}
				else {
					fwrite(block_data,sizeof(char),strlen(block_data),output);
				}

				//count number of blocks read for error purposes
				count++;

				//update to next block
				block = FAT[block];

			} while(block != FAT_EOF);

			if (fclose(output) != 0){
				fprintf(stderr,"Error closing output file\n");
				exit(0);
			}

			if (count != num_blocks){
				fprintf(stderr,"Error. Number of blocks read doesn't match number from directory\nNumber read: %d\n",count);

			}

			break;
		}
	}	
	if (!flag){
		printf("File not found.\n");
	}

	if (fclose(input) != 0){
		fprintf(stderr,"Error closing files\n");
	}

	return 0;
}

void print_block(int bytes[]){
	int i,space = 0,newline = 0;
	for (i=0;i<DEFAULT_BLOCK_SIZE;i++){
		//printf("%02x",bytes[i]);
		printf("%c",bytes[i]);
		//space++;
		if (space == 2){
			printf(" ");
			space = 0;
			newline++;
			if (newline == 8){
				printf("\n");
				newline = 0;
			}
		}
	}
}

//function to read the FAT table from the disk img
void FAT_load(FILE * input,int FAT[],int FAT_size,int FAT_Start){
		
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
