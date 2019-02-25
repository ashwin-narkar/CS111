#include <stdio.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <unistd.h>
#include <fcntl.h>

#include <stdint.h>

int block_count;
int inode_count;
int bsize;
int inode_size;
int blocks_per_group;
int inodes_per_group;

void superblockAnalysis(int image) {
	struct ext2_super_block* superblock;
	size_t bytesToRead = sizeof(struct ext2_super_block);
	superblock = (struct ext2_super_block*) malloc(sizeof(struct ext2_super_block));

	pread(image,superblock,bytesToRead,1024);
	bsize = EXT2_MIN_BLOCK_SIZE << superblock->s_log_block_size;
	inode_size = superblock->s_inode_size;
	printf("SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", superblock->s_blocks_count, superblock-> s_inodes_count,bsize,superblock->s_inode_size,superblock->s_blocks_per_group,superblock->s_inodes_per_group,superblock->s_first_ino);

	
	block_count = superblock->s_blocks_count;
	inode_count = superblock->s_inodes_count;
	blocks_per_group = superblock->s_blocks_per_group;
	inodes_per_group = superblock->s_inodes_per_group;
	free(superblock);

}

char getInodeFileType(struct ext2_inode* i) {
	uint16_t mode = i->i_mode;
	if (mode & 0x8000) {
		return 'f';
	}
	else if (mode & 0xA000) {
		return 's';
	}
	else if (mode & 0x4000) {
		return 'd';
	}
	else {
		return '?';
	}
}

void groupAnalysis(int image) {
	int num_groups = block_count / blocks_per_group;
	if (block_count % blocks_per_group != 0) {
		num_groups += 1;
	}
	struct ext2_group_desc* groupDescriptor;
	groupDescriptor = malloc(num_groups*sizeof(struct ext2_group_desc));
	pread(image,groupDescriptor,num_groups*sizeof(struct ext2_group_desc),1024+1024);
	int i;
	int total_blocks = ((num_groups-1) * blocks_per_group) + (block_count % blocks_per_group);
	int total_inodes = ((num_groups-1) * inodes_per_group) + (inode_count % inodes_per_group);
	int blocks_in_curr_group;
	int inodes_in_curr_group;
	for (i=0;i<num_groups;i++) {
		if (total_blocks > blocks_per_group) {
			blocks_in_curr_group = blocks_per_group;
		}
		else {
			blocks_in_curr_group = total_blocks;
		}
		if (total_inodes > inodes_per_group) {
			inodes_in_curr_group = inodes_per_group;
		}
		else {
			inodes_in_curr_group = total_inodes;
		}
		printf("GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",i,blocks_in_curr_group,inodes_per_group,groupDescriptor->bg_free_blocks_count,groupDescriptor->bg_free_inodes_count,groupDescriptor->bg_block_bitmap,groupDescriptor->bg_inode_bitmap,groupDescriptor->bg_inode_table);
		total_blocks -= blocks_per_group;
		total_inodes -= inodes_per_group;

		//Next part in group
		//Analyze the block bitmap
		//Size of block divided by 8 is the size of the bitmap

		//get the location by finding blockid*blocksize
		//the offset is blockid*bsize
		int bitmapsize = blocks_per_group / 8;
		//use uint_8 to store bytes and then read them. there should be bitmapsize of them
		uint8_t* bitmap = malloc(bitmapsize*sizeof(uint8_t));
		pread(image,bitmap,bitmapsize,groupDescriptor->bg_block_bitmap*bsize);
		int j;
		int k;
		for (j=0;j<bitmapsize;j++) {
			uint8_t currByte = bitmap[j];

			for (k=0;k<8;k++){
				//0 means free
				//1 means allocated
				if (((currByte>>k) & 0x01) == 0) {
					//this byte is free
					//byte is j*8 + k
					printf("BFREE,%d\n",j*8 + k +1 );
				} 
			}
		}
		//same thing for inodes
		int inode_bitmap_size = inodes_per_group / 8;
		uint8_t* inode_bitmap = malloc(inode_bitmap_size * sizeof(uint8_t));
		pread(image,inode_bitmap,inode_bitmap_size,groupDescriptor->bg_inode_bitmap*bsize);
		for (j=0;j<inode_bitmap_size;j++) {
			uint8_t currByte = inode_bitmap[j];
			for (k=0;k<8;k++){
				//0 means free
				//1 means allocated
				if (((currByte>>k) & 0x01) == 0) {
					//this byte is free
					//byte is j*8 + k
					printf("IFREE,%d\n",j*8 + k +1 );
				} 
				else {
					This inode isnt free so scan it
					int curr_inode_num = j*8 + k;
					struct ext2_inode* curr_inode = (struct ext2_inode*)malloc(sizeof(struct ext2_inode));
					//read the inode at this number since it is allocated
					//inode size is basically size of the inode struct
					pread(image,curr_inode,sizeof(struct ext2_inode),(curr_inode_num*sizeof(struct ext2_inode)) + (groupDescriptor->bg_inode_table*bsize));
					//find file type

					printf("INODE,%d,%c\n",curr_inode_num,getInodeFileType(&curr_inode));	
				}
			}
		}




	}

}


int main(int argc, char* argv[]) {
	//read superblock data
	
	if (argc != 2) {
		fprintf(stderr, "Invalid argument number\n");
	}

	char* filename = argv[1];

	int image = open(filename,O_RDONLY);
	superblockAnalysis(image);
	groupAnalysis(image);
}	
