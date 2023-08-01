// CSC360 Assignment 3 - Part 2
// Author: Adam Page
// Date: August 1: 2020
//
// FAT-like File system
// This program displays the contents of the root directory in the supplied file system (".img")
// Arguments: [file system]
//   file system - a ".img" file system file
//


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Constants.h"






void print_block(int bytes[]);
void exersize_print(char * filename);


int main(int argc,char * argv[]){
	if (argc != 2)	{
		fprintf(stderr,"Error please provide one paramenter. A disk image file\n");
		exit(0);
	}

	FILE *input;

	if ( (input = fopen(argv[1],"r") ) == NULL) {
		fprintf(stderr,"Error opening file: %s\n",argv[1]);
	}
	
	//define some variables we'll need later
	int i;

	//Byte location of Root Dir
	int Root_Dir_Start = 0;
	fseek(input,ROOTDIRSTART_OFFSET,SEEK_SET);
	int temp_arr[ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET];
	for (i=0;i<ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET;i++){
		temp_arr[i] = fgetc(input);
		Root_Dir_Start += temp_arr[i] << (8*(ROOTDIRBLOCKS_OFFSET-ROOTDIRSTART_OFFSET-i-1));
	}
	Root_Dir_Start *= DEFAULT_BLOCK_SIZE;

	//Read Root Dir into array for processing
	int bytes[DEFAULT_BLOCK_SIZE];
	fseek(input,Root_Dir_Start,SEEK_SET);
	for (i=0 ; i < DEFAULT_BLOCK_SIZE ; i++){
		bytes[i] = (int)fgetc(input); 
	}			


	//process root dir block
	int j = 0,k,temp;
	for (i = 0 ; i < DEFAULT_BLOCK_SIZE ; i += DIRECTORY_ENTRY_SIZE){ //8 entries start each iteration at the status
		//status
		j = i;
		temp = 0b00000110;
		temp = temp & bytes[j++];
		if (temp == 0b00000100){
			printf("D ");
		}
		else if (temp == 0b00000010){
			printf("F ");
		}
		else{
			continue;
		}


		//file size
		j = i;
		j += DIRECTORY_FILE_SIZE_OFFSET;
		temp = 0;
		int offset = DIRECTORY_CREATE_OFFSET - DIRECTORY_FILE_SIZE_OFFSET;
		for (k=0; k < offset; k++){
			temp += bytes[j++]*pow(16,(offset-1)*2 - k*2);
		}
		printf("%10d ",temp);


		//file name
		j = i;
		j += DIRECTORY_FILENAME_OFFSET;
		temp = 0;
		char name[DIRECTORY_MAX_NAME_LENGTH];
		for (k=0 ; k<DIRECTORY_MAX_NAME_LENGTH ; k++){
			name[k] = ' ';
		}
		int len = 0;
		while(bytes[j+1] != 0){
			len++;
			j++;
		}
		len++;
		for (k=DIRECTORY_MAX_NAME_LENGTH-1; k > DIRECTORY_MAX_NAME_LENGTH - len - 1; k--){
			name[k] = bytes[j--];
		}
		printf("%s ",name);
		

		//modification date
		j = i;
		j += DIRECTORY_MODIFY_OFFSET;
		
		//printf("%x %x ",bytes[j],bytes[j+1]);
		printf("%04d/",(int)(bytes[j++]*pow(16,2) + bytes[j++]*pow(16,0))); //year
		printf("%02d/",bytes[j++]); //month
		printf("%02d ",bytes[j++]); //day
		printf("%02d:",bytes[j++]); //hour
		printf("%02d:",bytes[j++]); //minutes
		printf("%02d",bytes[j++]); //seconds

		printf("\n");
	}	

	if (fclose(input) != 0){
		fprintf(stderr,"Error closing input file\n");
		exit(0);
	}
	return 0;
}