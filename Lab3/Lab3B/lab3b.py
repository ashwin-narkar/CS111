#!/usr/bin/python
import csv
import sys


#csv entries
#superblock
#SUPERBLOCK,total_blocks,total_inodes,block_size,inode_size,block/group,inode/group,first_nonreserved_inode

#group
#GROUP,groupnumber,total_blocks_in_group,total_inodes_in_group,num_free_blocks,num_free_inodes,freeblockbitmap,freeinodebitmap,firstinodeblock

#free block
#BFREE,blocknumber

#free inode
#IFREE,inodenumber

#inode summary
#INODE,number,filetype,mode,owner,group,linkcount,lastchange,modtime,accesstime,filesize,bptrs[15]

#directory entries
#DIRENT,parentinode,offset,inodenumber,...

#indirect entries
#INDIRECT,inodenum,level,offset,blocknumber_of_indirect

superblockdata = []
groupdata = []
freeblocks = []
freeinodes = []
directories = []
inodes = []
indirectblocks = []


#build up two arrays
inode_free_bitmap = []
block_free_bitmap = []
total_blocks=0
firstlegalblock=0
total_inodes=0
reserved_inode=0
block_state = []
inode_state = []
link_count = []
parent = []				#directory entries



def isLegalBlock(b):
	# b is block number
	# if block number is less than 0
	# if it is greater than or equal to total number of blocks in the group
	# less than first legal block
	if (b<0 or b>=total_blocks or b<firstlegalblock):
		return False
	else:
		return True

def isFreeBlock(b):
	if (isLegalBlock(b) == False):
		return False
	else:
		return block_free_bitmap[b]

def isFreeInode(i):
	if (i>2 and i < reserved_inode or i > total_inodes):
		return False
	else:
		return inode_free_bitmap[i]

def block_state_fill():
	for b in range(total_blocks):
		block_state.append("None")
	for b in range(total_blocks):
		if (isLegalBlock(b) == False):
			block_state[b] = "RESERVED"
		elif (isFreeBlock(b)):
			block_state[b] = "FREE"

def inodeInCSV(inode):
	for i in range(total_inodes):
		if (inode==i):
			return True
	return False

def inode_statefill():
	for inode_i in inodes:
		i_mode = int(inode_i[3])
		
		if (inodeInCSV(int(inode_i[1])-1) == False):
			inode_state[int(inode_i[1])-1] = 0
		elif (i_mode == 0):			#EMPTY INODE
			inode_state[int(inode_i[1])-1] = 0
		else:
			inode_state[int(inode_i[1])-1] = int(inode_i[6])		#SET STATE TO NUMBER OF HARD LINKS
	

def inodeEval():


	for i in inodes:
		inode = int(i[1])-1
		free = isFreeInode(inode)
		status = int(inode_state[inode])
		lcount = link_count[inode]
		if (free and status == 0):
			continue
		elif (free and status != 0):
			print "ALLOCATED INODE "+ i[1] + " ON FREELIST"
		elif (free == False and status == 0 and inode>=reserved_inode):
			print free
			print status
			#print "ALLOCATED INODE " + i[1] + " ON FREELIST"
		elif (status != lcount):
			print "INODE " + i[1] + " HAS " + str(lcount) + " LINKS BUT LINKCOUNT IS " + str(status)

def scanDirectories():
	for d in (directories):
		if (int(d[3]) > total_inodes):
			print "DIRECTORY INODE " + d[1] + " NAME " + d[6] + " INVALID INODE " + d[3]
		elif (inode_state[int(d[3])-1] == 0):
			print "DIRECTORY INODE " + d[1] + " NAME " + d[6] + " UNALLOCATED INODE " + d[3]
		else:
			link_count[int(d[3])-1] += 1
			parent[int(d[3])] = d[6]
		if ((d[6] == "'.'") and (int(d[1]) != int(d[3]))):
			print "DIRECTORY INODE " + d[1] + " NAME " + d[6] + " LINK TO INODE " + d[3] + " SHOULD BE " + d[1]
		if ((d[6] == "'..'") and (int(d[3]) > int(d[1]))):
			print "DIRECTORY INODE " + d[1] + " NAME " + d[6] + " LINK TO INODE " + d[3] + " SHOULD BE " + d[1]
		

	

def checkBlock(b):
	if (b<0 or b>=total_blocks):
		return "INVALID"
	else:
		return block_state[b]


def findDuplicate(d):
	for i in inodes:
		for j in range(12,27):
			b = int(i[j])
			if (b == d):
				return i[1]
	return -1

def scanInodes():
	lastDuplicate = ""
	for i in range(total_inodes):
		if (inode_state[i] == 0):
			continue
		
		curr_inode_num = -1
		for j in range(len(inodes)):
			if (int(inodes[j][1]) == i+1):
				curr_inode_num = j
		
		if (curr_inode_num < 0):
			continue
		for j in range(12,24):
			bptr = int(inodes[curr_inode_num][j])
			if (bptr != 0):
				status=checkBlock(bptr)
				if (status == "None"):
					block_state[bptr] = "USED"
					lastDuplicate = "DUPLICATE BLOCK " + str(inodes[curr_inode_num][j]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 0"
				elif (status == "USED"):
					lastDuplicate= findDuplicate(bptr)
					print "DUPLICATE BLOCK " + str(inodes[curr_inode_num][j]) + " IN INODE " + str(lastDuplicate) + " AT OFFSET 0"
					print "DUPLICATE BLOCK " + str(inodes[curr_inode_num][j]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 0"
					block_state[bptr] == "DUPLICATE"
				elif (status == "DUPLICATE"):
					print "DUPLICATE BLOCK " + str(inodes[curr_inode_num][j]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 0"
				elif (status == "RESERVED"):
					print "RESERVED BLOCK " + str(inodes[curr_inode_num][j]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 0"
				elif (status == "INVALID"):
					print "INVALID BLOCK " + str(inodes[curr_inode_num][j]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 0"
				elif (block_free_bitmap[bptr] == True):
					print "ALLOCATED BLOCK " + str(bptr) + " ON FREELIST"
			
		bptr = int(inodes[curr_inode_num][24])
		if (bptr != 0):
			status=checkBlock(bptr)
			if (status == "None"):
				block_state[bptr] = "USED"
			elif (status == "USED"):
				lastDuplicate= findDuplicate(bptr)
				print "DUPLICATE BLOCK " + str(inodes[curr_inode_num][24]) + " IN INODE " + str(lastDuplicate) + " AT OFFSET 0"
				print "DUPLICATE INDIRECT BLOCK " + str(inodes[curr_inode_num][24]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 12"
				block_state[bptr] == "DUPLICATE"
			elif (status == "DUPLICATE"):
				print "DUPLICATE INDIRECT BLOCK " + str(inodes[curr_inode_num][24]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 12"
			elif (status == "RESERVED"):
				print "RESERVED INDIRECT BLOCK " + str(inodes[curr_inode_num][24]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 12"
			elif (status == "INVALID"):
				print "INVALID INDIRECT BLOCK " + str(inodes[curr_inode_num][24]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 12"	
			elif (block_free_bitmap[bptr] == True):
					print "ALLOCATED BLOCK " + str(bptr) + " ON FREELIST"
		bptr = int(inodes[curr_inode_num][25])
		if (bptr != 0):
			status=checkBlock(bptr)
			if (status == "None"):
				block_state[bptr] = "USED"
			elif (status == "USED"):
				lastDuplicate= findDuplicate(bptr)
				print "DUPLICATE BLOCK " + str(inodes[curr_inode_num][25]) + " IN INODE " + str(lastDuplicate) + " AT OFFSET 0"
				print "DUPLICATE DOUBLE INDIRECT BLOCK " + str(inodes[curr_inode_num][25]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 268"
				block_state[bptr] == "DUPLICATE"
			elif (status == "DUPLICATE"):
				print "DUPLICATE DOUBLE INDIRECT BLOCK " + str(inodes[curr_inode_num][25]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 268"
			elif (status == "RESERVED"):
				print "RESERVED DOUBLE INDIRECT BLOCK " + str(inodes[curr_inode_num][25]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 268"
			elif (status == "INVALID"):
				print "INVALID DOUBLE INDIRECT BLOCK " + str(inodes[curr_inode_num][25]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 268"	
			elif (block_free_bitmap[bptr] == True):
					print "ALLOCATED BLOCK " + str(bptr) + " ON FREELIST"
		bptr = int(inodes[curr_inode_num][26])
		if (bptr != 0):
			status=checkBlock(bptr)
			if (status == "None"):
				block_state[bptr] = "USED"
			elif (status == "USED"):
				lastDuplicate= findDuplicate(bptr)
				print "DUPLICATE BLOCK " + str(inodes[curr_inode_num][26]) + " IN INODE " + str(lastDuplicate) + " AT OFFSET 0"
				print "DUPLICATE TRIPLE INDIRECT BLOCK " + str(inodes[curr_inode_num][26]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 65804"
				block_state[bptr] == "DUPLICATE"
			elif (status == "DUPLICATE"):
				print "DUPLICATE TRIPLE INDIRECT BLOCK " + str(inodes[curr_inode_num][26]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 65804"
			elif (status == "RESERVED"):
				print "RESERVED TRIPLE INDIRECT BLOCK " + str(inodes[curr_inode_num][26]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 65804"
			elif (status == "INVALID"):
				print "INVALID TRIPLE INDIRECT BLOCK " + str(inodes[curr_inode_num][26]) + " IN INODE " + str(inodes[curr_inode_num][1]) + " AT OFFSET 65804"			
			elif (block_free_bitmap[bptr] == True):
					print "ALLOCATED BLOCK " + str(bptr) + " ON FREELIST"




def main():
	if (len(sys.argv) != 2):
		print "Error. Invalid argument number"
		exit(1)
	csvfilename=sys.argv[1]
	csvdata = []
	with open(csvfilename,'r') as csvfile:
		reader = csv.reader(csvfile, delimiter=',',quotechar='|')
		for row in reader:
			csvdata.append(row)

	for row in csvdata:
		if (row[0] == 'SUPERBLOCK'):
			superblockdata.append(row)
		if (row[0] == 'GROUP'):
			groupdata.append(row)
		if (row[0] == 'BFREE'):
			freeblocks.append(row)
		if (row[0] == 'IFREE'):
			freeinodes.append(row)
		if (row[0] == 'INDIRECT'):
			indirectblocks.append(row)
		if (row[0] == 'DIRENT'):
			directories.append(row)
		if (row[0] == 'INODE'):
			inodes.append(row)
	
	global total_blocks 
	total_blocks = int(superblockdata[0][1])
	global total_inodes 
	total_inodes = int(superblockdata[0][2])
	#(total number of inodes * inode size) / (block size). 
	inode_size = int(superblockdata[0][4])
	bsize = int(superblockdata[0][3])
	global firstlegalblock
	firstlegalblock=4+(total_inodes*inode_size/bsize)

	global reserved_inode
	reserved_inode = int(superblockdata[0][7])
	for i in range(total_blocks):
		block_free_bitmap.append(False)
	for i in range(total_inodes):
		inode_free_bitmap.append(False)
	for row in freeblocks:
		block_free_bitmap[int(row[1])] = True
	for row in freeinodes:
		inode_free_bitmap[int(row[1])-1] = True
	
	block_state_fill()



	for i in range(total_inodes):
		inode_state.append(0)
		link_count.append(0)
		parent.append(0)
		
	
	inode_statefill()
	
	scanInodes()
	scanDirectories()

	inodeEval()

	for i in range(reserved_inode, total_inodes):
		if (inode_free_bitmap[i] == False and inode_state[i] == 0):
			print "UNALLOCATED INODE " + str(i+1) + " NOT ON FREELIST"
	
	for indirect in indirectblocks:
		block_num = int(indirect[5])
		block_state[block_num] = "USED"
	for i in range(firstlegalblock +1,len(block_state)):
		if (block_state[i] == "None"):
			print "UNREFERENCED BLOCK " + str(i)

if __name__== "__main__":
	main()


