#include <stdio.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
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

	if (pread(image,superblock,bytesToRead,1024) == -1) {
		fprintf(stderr, "Could not read superblock\n");
		exit(2);
	}
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


void directoryAnalysis(int image, int parent_inode, int blockOffset) {
	
	//scan each data block in a directory inode
	//first 12 are direct
	//read the offset
	//each directory is block size * offset
	//namelen maybe variable but always read the directory struct
	//print out only first namelen chars
	//use reclen to add to offset
	int offsetToNextDir = 0;
	
	int dir_entry = 0;
	while (offsetToNextDir < bsize) {
		struct ext2_dir_entry *dir = malloc(sizeof(struct ext2_dir_entry));
		if (pread(image,dir,sizeof(struct ext2_dir_entry),blockOffset*bsize + offsetToNextDir) == -1) {
			fprintf(stderr, "Unable to read directory\n");
			exit(2);
		}
		dir_entry++;
		if (dir->inode == 0) {
			//directory entry is not used
			offsetToNextDir += dir->rec_len;
			continue;
		}
		//in other cases
		//add nullbyte for the name at namelen in the char array
		dir->name[dir->name_len] = '\0';
		//print out directory information
		printf("DIRENT,%d,%d,%d,%d,%d,\'%s\'\n",parent_inode+1,offsetToNextDir,dir->inode,dir->rec_len,dir->name_len,dir->name);
		offsetToNextDir += dir->rec_len;
		free(dir);
	}
	
}


void groupAnalysis(int image) {
	int num_groups = block_count / blocks_per_group;
	if (block_count % blocks_per_group != 0) {
		num_groups += 1;
	}
	struct ext2_group_desc* groupDescriptor;
	groupDescriptor = malloc(num_groups*sizeof(struct ext2_group_desc));
	if (pread(image,groupDescriptor,num_groups*sizeof(struct ext2_group_desc),1024+1024) == -1) {
		fprintf(stderr, "Unable to read groupDescriptor\n");
		exit(2);
	}
	int i;
	int total_blocks = ((num_groups-1) * blocks_per_group) + (block_count % blocks_per_group);
	int total_inodes = ((num_groups-1) * inodes_per_group) + (inode_count % inodes_per_group);
	int blocks_in_curr_group;
	
	for (i=0;i<num_groups;i++) {
		if (total_blocks > blocks_per_group) {
			blocks_in_curr_group = blocks_per_group;
		}
		else {
			blocks_in_curr_group = total_blocks;
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
		if (pread(image,bitmap,bitmapsize,groupDescriptor->bg_block_bitmap*bsize) == -1) {
			fprintf(stderr, "Unable to read block bitmap\n");
			exit(2);
		}
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
		if (pread(image,inode_bitmap,inode_bitmap_size,groupDescriptor->bg_inode_bitmap*bsize) == -1) {
			fprintf(stderr, "Unable to read inode bitmap\n");
			exit(2);
		}
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
					// This inode isnt free so scan it
					int curr_inode_num = j*8 + k;
					struct ext2_inode* curr_inode = (struct ext2_inode*)malloc(sizeof(struct ext2_inode));
					//read the inode at this number since it is allocated
					//inode size is basically size of the inode struct
					if (pread(image,curr_inode,sizeof(struct ext2_inode),(curr_inode_num*sizeof(struct ext2_inode)) + (groupDescriptor->bg_inode_table*bsize)) == -1) {
						fprintf(stderr, "Unable to read inode\n");
						exit(2);
					}
					//find file type
					if (curr_inode->i_links_count == 0) {
						continue;
					}
					long int ctime = curr_inode->i_ctime;
					long int mtime = curr_inode->i_mtime;
					long int atime = curr_inode->i_atime;
					struct tm* last_change = gmtime(&ctime);
					
					
					
					printf("INODE,%d,%c,%o,%d,%d,%d,",curr_inode_num+1,getInodeFileType(curr_inode),(curr_inode->i_mode & 0xFFF),curr_inode->i_uid,curr_inode->i_gid,curr_inode->i_links_count);	
					//Now print times in mm/dd/yy hh:mm:ss
					printf("%02d/%02d/%02d %02d:%02d:%02d,", last_change->tm_mon +1 , last_change->tm_mday, (last_change->tm_year)%100,last_change->tm_hour, last_change->tm_min, last_change->tm_sec);
					struct tm* mod_time = gmtime(&mtime);
					printf("%02d/%02d/%02d %02d:%02d:%02d,", mod_time->tm_mon +1 , mod_time->tm_mday, (mod_time->tm_year)%100,mod_time->tm_hour, mod_time->tm_min, mod_time->tm_sec);
					struct tm* last_access = gmtime(&atime);
					printf("%02d/%02d/%02d %02d:%02d:%02d,", last_access->tm_mon +1 , last_access->tm_mday, (last_access->tm_year)%100,last_access->tm_hour, last_access->tm_min, last_access->tm_sec);					
					printf("%d,%d",curr_inode->i_size,curr_inode->i_blocks);
					//printout block pointers
					if (getInodeFileType(curr_inode) == 's' && curr_inode->i_size > 60) {
						//dont print
					}
					else {
						//print the next 15 block pointers
						int x;
						for (x=0;x<EXT2_N_BLOCKS;x++) {
							printf(",%d",curr_inode->i_block[x]);
						}
						printf("\n");
					}
					int y;
					//If directory do teh directory stuff
					if (getInodeFileType(curr_inode) == 'd' || getInodeFileType(curr_inode) == 'f') {
						//evaluate indirect block
						int num_indirect_blocks = bsize / 4; 
						//each indirect block id is actually 4 blocks
						int singleIndirectBlockOffset = curr_inode->i_block[12];
						//printf("single indirect blocks from inode %d have value %d\n",curr_inode_num+1,singleIndirectBlockOffset);
						if (singleIndirectBlockOffset != 0) {
							//we have indirect block
							uint32_t *indirectBlocks = malloc(num_indirect_blocks*sizeof(uint32_t));
							//4 bytes each address divided by blocksize
							if (pread(image,indirectBlocks,bsize,bsize*singleIndirectBlockOffset) == -1) {
								fprintf(stderr, "Unable to read single indirect block\n");
								exit(2);
							}
							//each of the things now is basically like a new offset to a directory
							for (y=0;y<num_indirect_blocks;y++) {	//reusing y hehe
								if (indirectBlocks[y] != 0) {
									printf("INDIRECT,%d,1,%d,%d,%d\n",curr_inode_num+1,y+12,curr_inode->i_block[12],indirectBlocks[y]);
									if (getInodeFileType(curr_inode) == 'd') {
										directoryAnalysis(image,curr_inode_num,indirectBlocks[y]);
									}
								}
								
							}
							free(indirectBlocks);
						}
						int doubleIndirectBlockOffset = curr_inode->i_block[13];
						if (doubleIndirectBlockOffset != 0) {
							//each of these values is an indirect block offset basically
							uint32_t *doubleindirectBlocks = malloc(num_indirect_blocks*sizeof(uint32_t));
							if (pread(image,doubleindirectBlocks,bsize,bsize*doubleIndirectBlockOffset) == -1) {
								fprintf(stderr, "Unable to read double indirect block\n");
								exit(2);
							}
							//basically now for each of these, do exactly what u did for an indirect block if its not 0
							int a;
							for (a=0;a<num_indirect_blocks;a++) {
								if (doubleindirectBlocks[a] != 0) {
									printf("INDIRECT,%d,2,%d,%d,%d\n",curr_inode_num+1,268+(a),curr_inode->i_block[13],doubleindirectBlocks[a]);
									//this is the new singleindirectblockoffset and do the same thing
									singleIndirectBlockOffset = doubleindirectBlocks[a];
									if (singleIndirectBlockOffset != 0) {
										//we have indirect block
										uint32_t *indirectBlocks = malloc(num_indirect_blocks*sizeof(uint32_t));
										//4 bytes each address divided by blocksize
										pread(image,indirectBlocks,bsize,bsize*singleIndirectBlockOffset);
										//each of the things now is basically like a new offset to a directory
										for (y=0;y<num_indirect_blocks;y++) {	//reusing y hehe
											if (indirectBlocks[y] != 0) {
												printf("INDIRECT,%d,1,%d,%d,%d\n",curr_inode_num+1,268+(255*a)+y,doubleindirectBlocks[a],indirectBlocks[y]);
												if (getInodeFileType(curr_inode) == 'd') {
													directoryAnalysis(image,curr_inode_num,indirectBlocks[y]);
												}
											}
											
										}
										free(indirectBlocks);
									}
								}
							}
							free(doubleindirectBlocks);
						}
						int tripleIndirectBlockOffset = curr_inode->i_block[14];
						if (tripleIndirectBlockOffset != 0) {
							uint32_t *tripleindirectBlocks = malloc(num_indirect_blocks*sizeof(uint32_t));
							pread(image,tripleindirectBlocks,bsize,bsize*tripleIndirectBlockOffset);
							//for each of these, they are doubly indirect
							int b;
							for (b=0;b<num_indirect_blocks;b++) {
								if (tripleindirectBlocks[b] != 0) {
									printf("INDIRECT,%d,3,%d,%d,%d\n",curr_inode_num+1,12+256 +b+(256*256),curr_inode->i_block[14],tripleindirectBlocks[b]);	
									//each of these is a double block	
									doubleIndirectBlockOffset = tripleindirectBlocks[b];
									if (doubleIndirectBlockOffset != 0) {
										//each of these values is an indirect block offset basically
										uint32_t *doubleindirectBlocks = malloc(num_indirect_blocks*sizeof(uint32_t));
										pread(image,doubleindirectBlocks,bsize,bsize*doubleIndirectBlockOffset);
										//basically now for each of these, do exactly what u did for an indirect block if its not 0
										int a;
										for (a=0;a<num_indirect_blocks;a++) {
											if (doubleindirectBlocks[a] != 0) {
												printf("INDIRECT,%d,2,%d,%d,%d\n",curr_inode_num+1,12+256 +b+(256*256),tripleindirectBlocks[b],doubleindirectBlocks[a]);
												//this is the new singleindirectblockoffset and do the same thing
												singleIndirectBlockOffset = doubleindirectBlocks[a];
												if (singleIndirectBlockOffset != 0) {
													//we have indirect block
													uint32_t *indirectBlocks = malloc(num_indirect_blocks*sizeof(uint32_t));
													//4 bytes each address divided by blocksize
													pread(image,indirectBlocks,bsize,bsize*singleIndirectBlockOffset);
													//each of the things now is basically like a new offset to a directory
													for (y=0;y<num_indirect_blocks;y++) {	//reusing y hehe
														if (indirectBlocks[y] != 0) {
															printf("INDIRECT,%d,1,%d,%d,%d\n",curr_inode_num+1,y+12+256 +b+(256*256),doubleindirectBlocks[a],indirectBlocks[y]);
															if (getInodeFileType(curr_inode) == 'd') {
																directoryAnalysis(image,curr_inode_num,indirectBlocks[y]);
															}
														}
													}
													free(indirectBlocks);
												}
											}
										}
										free(doubleindirectBlocks);
									}
								}

							}
							free(tripleindirectBlocks);
						}
						if (getInodeFileType(curr_inode) == 'd') {
							for (y=0;y<12;y++) {
								int blockOffset = curr_inode->i_block[y];
								if (blockOffset == 0) { //A value of 0 in this array effectively terminates it with no further block being defined.	All the remaining entries of the array should still be set to 0.
									break;
								}
								directoryAnalysis(image,curr_inode_num,blockOffset);
							}
						}
						
					}


					
					free(curr_inode);
				}
			}

		}
		free(inode_bitmap);
		free(bitmap);
	}

}


int main(int argc, char* argv[]) {
	//read superblock data
	
	if (argc != 2) {
		fprintf(stderr, "Invalid argument number\n");
		exit(1);
	}

	char* filename = argv[1];

	int image = open(filename,O_RDONLY);
	if (image == -1) {
		fprintf(stderr, "Unable to open file. Bad argument\n");
		exit(1);
	}
	superblockAnalysis(image);
	groupAnalysis(image);
	exit(0);
}	
