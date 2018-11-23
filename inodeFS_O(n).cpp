#include <stdio.h>
#include <bits/stdc++.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
using namespace std;

#define DISK_BLOCKS  40960  //disk_blks  =128k = 131072
#define BLOCK_SIZE   4096       //blk_size = 4096
#define NO_OF_INODES 512  //16*1024 = 16384
#define NO_OF_FILEDESCRIPTORS 32  //No of File descriptors can be initialized here
#define FS fileDescriptor_map[fildes].second
//db=data block

int create_disk(); //to create disk with size = DISK_BLOCKS * BLOCK_SIZE
int user_inputs(); //to take usr input for disk name, DISK_BLOCKS, BLOCK_SIZE
int mounting(); //taking metadata into data St
int unmounting(); //saving all the changes and closing disk
int create_file(char* filename);
int open_file(char* filename);  //assign fd to given filename & set cur pos to 0 and return fd
int close_file(int fd);
int delete_file(char *filename); //delete filename completly from disk i.e free DB and free inode
int read_file(int fd,char* buf,int nbytes);	//To read file and copy the whole content into a file at Desktop / PWD


//Helper functions for open_file
int read_block(int blockNo,char* buf);	//To read Block_SIZE amount of data from disk
int write_block(int block, char *buf, int size_to_wrt, int start_pos_to_wrt); //in gen write_block(5, buf, 4096, 0) i.e write all data(of 4096 size) into DB no=5 starting from 0th pos
int write_file(int fd, char * buf, int size); //writes 'size' no of bytes from buf into file pointed by fd from cur_pos mentioned in fd
int store_file_into_Disk(char * filename); //take input filename and store into disk using create_file() & write_file() function.








struct inode
{
	int inode_num;
	int type;
	int filesize;
	int pointer[13]; //contains DB nos of files (-1 if no files assigned)
};





struct dir_entry
{
	char file_name[20]; //filename size is 19 max
	int inode_num;  // of this
};





struct super_block  /// map of directory  structure
{
	//int version;
			/* directory information */
	int starting_db_of_dir_st= ceil( ((float)sizeof(super_block))/BLOCK_SIZE );    //33; //set in main or create_disk = ceil( (float)sizeof(super_block)/block_size )
	int no_of_db_in_dir_st= (sizeof(struct dir_entry)*NO_OF_INODES)/BLOCK_SIZE;
			/* data information */
	int starting_inodes_db= starting_db_of_dir_st+no_of_db_in_dir_st; //130; //coz 0to32=SB 33to129=DS 130to(130+255)==130to385 inode data blocks
	int inode_blocks_contained_in_thisNoOf_DB= ceil( ((float)(NO_OF_INODES * sizeof(struct inode))) / BLOCK_SIZE );  //256; //coz tot no of inodes are 16k=16*1024 which is contained inside 256 DB
	int db_starting_index= starting_inodes_db + inode_blocks_contained_in_thisNoOf_DB;  //386; //coz 130to385 is inode blocks
	int no_of_db= DISK_BLOCKS - db_starting_index;  //130686; //coz 128k=128*1024 is total no of db in virtual disk out of this 386 assigned to meta data so 128k-386 = 130686 db for data
	char inode_freelist[NO_OF_INODES];  //to check which inode no is free to assign to file
	int next_freeinode=0;//index into inode_freelist[16384];  //index of inode_arr
	char datablock_freelist[DISK_BLOCKS];  //to check which DB is free to allocate to file
	unsigned int next_freedatablock=db_starting_index;  //the DB no which is free
	//add int free_avail_db
};



char disk_name[50],filename[20];
struct super_block sb;
struct dir_entry dir_structure[NO_OF_INODES];
struct inode inode_arr[NO_OF_INODES]; //array of all the inode num

FILE *fp;

map <string,int> dir_map;//file name as key maps to inode (value)
map <string,int> :: iterator it; //to iterate thru dir_map
vector<int> freeInodeVec; // inode nums of all free inodes
vector<int> freeDBvec; // db num of all free db

vector<int> freeFileDescriptorsVec;   //Keeps track of Free file descriptors. (max NO_OF_FILEDESCRIPTORS can be allowed.)
int openfile_count=0 ; //keeps track of number of files opened.
map <int,pair<int,int> > fileDescriptor_map;	//Stores files Descriptor as key and corresponding Inode number(First) and file pointer i.e cursor pos (second) as value













//-----------------------------------------------------------------------------------------------
//all the functions
//-----------------------------------------------------------------------------------------------

void clear_stdin()
{
	int c;
	do {
			c = getchar();
	} while (c != '\n' && c != EOF);
}

int user_inputs()
{
	int choice;
	printf("Enter /path/to/DiskName to create(/open existing) Virtual Disk: \n");
	scanf("%s",disk_name);

	if( access( disk_name, F_OK ) != -1 )
	{
	    // file exists
	    printf("The Virtual Disk already exists, Mounting now\n");
	    mounting();
	}
	else
	{

	    create_disk();
	    mounting();
	}

	while(1)
	{
		printf("\n1 - Create empty file into disk\n");
		printf("2 - Open a file from disk\n");
		printf("3 - Read a file from disk\n");
		printf("4 - Write a file into disk\n");
		printf("5 - Delete a file from disk\n");
		printf("6 - close a file using fd\n");
		printf("7 - List Stored files\n");
		printf("8 - Unmount the disk\n");


		scanf("%d",&choice);

		if(choice==1)
		{
			printf("Enter file name\n");
			//scanf("%s",filename);
			string temp;
			getline(cin,temp);
			char filename[20];
			strcpy(filename, temp.c_str());
			create_file(filename);
		}

		else if(choice == 2)
		{
			printf("Enter file name\n");
			// scanf("%s",filename);
			clear_stdin();
			string temp;
			getline(cin,temp);
			char filename[20];
			strcpy(filename, temp.c_str());
			open_file(filename);
		}
		else if(choice == 3)
		{
			int fddd;
			printf("Enter file Descriptor\n");
			scanf("%d",&fddd);
			char test_buf[BLOCK_SIZE];
			read_file(fddd,test_buf,4096);
		}
		else if(choice == 4)
		{
			printf("Enter source path/to/filename: ");
			//scanf("%s",filename);
			clear_stdin();

			string temp;
			getline(cin,temp);
			char filename[20];
			strcpy(filename, temp.c_str());
			store_file_into_Disk(filename);
		}
		else if(choice == 5)
		{
			printf("Enter filename to delete: ");
			//scanf("%s",filename);
			clear_stdin();

			string temp;
			getline(cin,temp);
			char filename[20];
			strcpy(filename, temp.c_str());
			delete_file(filename);
		}
		else if(choice == 6)
		{
			int fdd;
			printf("Enter file descriptor to close: ");
			scanf("%d",&fdd);
			close_file(fdd);
		}
		else if(choice == 7)
		{

			for ( it = dir_map.begin(); it!=dir_map.end(); it++) {
					printf("%s\n",(it->first).c_str());
			}
		}
		else if(choice == 8)
		{
			unmounting();
			return 0;
		}
	}

	return 1;
}

int create_disk()
{
	char buffer[BLOCK_SIZE];

	fp = fopen(disk_name,"wb");
	memset(buffer, 0, BLOCK_SIZE); //setting the buffer's value=NULL at all index

	for (int i = 0; i < DISK_BLOCKS; ++i)
		fwrite(buffer,1,BLOCK_SIZE,fp);
		//write(fp, buffer, BLOCK_SIZE);


	struct super_block sb;  //initializing sb

	//setting DB vecor array to 1(=used i.e cant assign to file) for all metadata DB and 0(=free) to all data DBs
	for (int i = 0; i < sb.db_starting_index; ++i)
		sb.datablock_freelist[i] = 1; //1 means DB is not free
	for (int i = sb.db_starting_index; i < DISK_BLOCKS; ++i)
		sb.datablock_freelist[i] = 0; //1 means DB is not free
	for(int i = 0; i < NO_OF_INODES; ++i)
		sb.inode_freelist[i]=0; // char 0;

	for (int i = 0; i < NO_OF_INODES; ++i)
	{
		inode_arr[i].pointer[0]=-1;
		inode_arr[i].pointer[1]=-1;
		inode_arr[i].pointer[2]=-1;
		inode_arr[i].pointer[3]=-1;
		inode_arr[i].pointer[4]=-1;
		inode_arr[i].pointer[5]=-1;
		inode_arr[i].pointer[6]=-1;
		inode_arr[i].pointer[7]=-1;
		inode_arr[i].pointer[8]=-1;
		inode_arr[i].pointer[9]=-1;
		inode_arr[i].pointer[10]=-1;
		inode_arr[i].pointer[11]=-1;
		inode_arr[i].pointer[12]=-1;
		//inode_arr[i].pointer[13]=-1;
	}


	//storing SB into begining of virtual disk
	fseek( fp, 0, SEEK_SET );
	char sb_buff[sizeof(struct super_block)];
	memset(sb_buff, 0, sizeof(struct super_block));
	memcpy(sb_buff,&sb,sizeof(sb));
	fwrite(sb_buff,1,sizeof(sb),fp);



	//storing DS after SB into virtual disk
	fseek( fp, (sb.starting_db_of_dir_st)*BLOCK_SIZE, SEEK_SET );
	char dir_buff[sizeof(dir_structure)];
	memset(dir_buff, 0, sizeof(dir_structure));
	memcpy(dir_buff,dir_structure,sizeof(dir_structure));
	fwrite(dir_buff,1,sizeof(dir_structure),fp);



	//storing inode DBs after SB & DB into virtual disk
	fseek( fp, (sb.starting_inodes_db)*BLOCK_SIZE, SEEK_SET );
	char inode_buff[sizeof(inode_arr)];
	memset(inode_buff, 0, sizeof(inode_arr));
	memcpy(inode_buff,inode_arr,sizeof(inode_arr));
	fwrite(inode_buff,1,sizeof(inode_arr),fp);



  	fclose(fp);
  	printf("\033[1;32mVirtual Disk '%s' created sucessfully\033[0m\n",disk_name );

	return 1;
}

int mounting()
{
	fp = fopen(disk_name,"rb+");
	//retrieve super block from virtual disk and store into global struct super_block sb
	char sb_buff[sizeof(sb)];
	memset(sb_buff,0,sizeof(sb));
	fread(sb_buff,1,sizeof(sb),fp);
	memcpy(&sb,sb_buff,sizeof(sb));


	//retrieve DS block from virtual disk and store into global struct dir_entry dir_structure[NO_OF_INODES]
	fseek( fp, (sb.starting_db_of_dir_st)*BLOCK_SIZE, SEEK_SET );
	char dir_buff[sizeof(dir_structure)];
	memset(dir_buff,0,sizeof(dir_structure));
	fread(dir_buff,1,sizeof(dir_structure),fp);
	memcpy(dir_structure,dir_buff,sizeof(dir_structure));



	//retrieve Inode block from virtual disk and store into global struct inode inode_arr[NO_OF_INODES];
	fseek( fp, (sb.starting_inodes_db)*BLOCK_SIZE, SEEK_SET );
	char inode_buff[sizeof(inode_arr)];
	memset(inode_buff,0,sizeof(inode_arr));
	fread(inode_buff,1,sizeof(inode_arr),fp);
	memcpy(inode_arr,inode_buff,sizeof(inode_arr));


	//storing all filenames into map
	for (int i = NO_OF_INODES - 1; i >= 0; --i)
		if(sb.inode_freelist[i]==1)
			dir_map[string(dir_structure[i].file_name)]=dir_structure[i].inode_num;
		else
			freeInodeVec.push_back(i);

	// populate freeinodevec and freedbvec
	for(int i = DISK_BLOCKS - 1; i>=sb.db_starting_index; --i)
		if(sb.datablock_freelist[i] == 0)
			freeDBvec.push_back(i);

	// Populate Free Filedescriptor vector
	for(int i=NO_OF_FILEDESCRIPTORS-1;i>=0;i--){
		freeFileDescriptorsVec.push_back(i);
	}

	printf("\033[1;36mDisk is mounted now\033[0m\n");


	/*printf(" Starting DB NO Directory Str%d\n", sb.starting_db_of_dir_st);
	printf(" Starting DB NO INODES%d\n", sb.starting_inodes_db);
	printf("NO Data blocks for directory structure %d\n",sb.no_of_db_in_dir_st);
	printf(" Data Blocks Starting index%d\n",sb.db_starting_index );
	printf(" Total No of data blocks%d\n",sb.no_of_db );*/
	return 1;
}

// int unmounting()
// {
// 	//to be done::
// 	//close all open fd
// 	//store bit vector for inode and datablock vector into SB then
//
// 	//storing SB into begining of virtual disk
// 	fseek( fp, 0, SEEK_SET );
// 	char sb_buff[sizeof(struct super_block)];
// 	memset(sb_buff, 0, sizeof(struct super_block));
// 	memcpy(sb_buff,&sb,sizeof(sb));
// 	fwrite(sb_buff,1,sizeof(sb),fp);
//
//
//
// 	//storing DS after SB into virtual disk
// 	fseek( fp, (sb.starting_db_of_dir_st)*BLOCK_SIZE, SEEK_SET );
// 	char dir_buff[sizeof(dir_structure)];
// 	memset(dir_buff, 0, sizeof(dir_structure));
// 	memcpy(dir_buff,dir_structure,sizeof(dir_structure));
// 	fwrite(dir_buff,1,sizeof(dir_structure),fp);
//
//
//
// 	//storing inode DBs after SB & DB into virtual disk
// 	fseek( fp, (sb.starting_inodes_db)*BLOCK_SIZE, SEEK_SET );
// 	char inode_buff[sizeof(inode_arr)];
// 	memset(inode_buff, 0, sizeof(inode_arr));
// 	memcpy(inode_buff,inode_arr,sizeof(inode_arr));
// 	fwrite(inode_buff,1,sizeof(inode_arr),fp);
//
//
//
// 	printf("Disk Unmounted\n");
// 	fclose(fp);
// 	return 1;
// }
//


int unmounting()
{

	//storing SB into begining of virtual disk
	for(int i = DISK_BLOCKS - 1; i>=sb.db_starting_index; --i)
		sb.datablock_freelist[i] == 1;
	for(int i=0;i<freeDBvec.size();i++){
		sb.datablock_freelist[freeDBvec.back()]=0;
		freeDBvec.pop_back();
	}
	//Initializing everyone to 1
	for (int i = 0; i < NO_OF_INODES; ++i)
	{
		sb.inode_freelist[i]=1;
	}
	//Making those inode nos which are free to 0.
	for (int i = 0; i < freeInodeVec.size(); ++i)
	{
		/* code */
		sb.inode_freelist[freeInodeVec.back()]=0;
		freeInodeVec.pop_back();
	}

	fseek( fp, 0, SEEK_SET );
	char sb_buff[sizeof(struct super_block)];
	memset(sb_buff, 0, sizeof(struct super_block));
	memcpy(sb_buff,&sb,sizeof(sb));
	fwrite(sb_buff,1,sizeof(sb),fp);



	//storing DS after SB into virtual disk
	fseek( fp, (sb.starting_db_of_dir_st)*BLOCK_SIZE, SEEK_SET );
	char dir_buff[sizeof(dir_structure)];
	memset(dir_buff, 0, sizeof(dir_structure));
	memcpy(dir_buff,dir_structure,sizeof(dir_structure));
	fwrite(dir_buff,1,sizeof(dir_structure),fp);



	//storing inode DBs after SB & DB into virtual disk
	fseek( fp, (sb.starting_inodes_db)*BLOCK_SIZE, SEEK_SET );
	char inode_buff[sizeof(inode_arr)];
	memset(inode_buff, 0, sizeof(inode_arr));
	memcpy(inode_buff,inode_arr,sizeof(inode_arr));
	fwrite(inode_buff,1,sizeof(inode_arr),fp);

	//close all open fd
	//store bit vector for inode and datablock vector

	printf("\033[1;36mDisk Unmounted\033[0m\n");
	fclose(fp);
	return 1;
}



int create_file(char* filename) //input: use global variable char filename[20] (already defined) to create a file and allocate one DB to this file
{
	int nextin, nextdb;
	//check if filename already exists in directory (using map dir_map)
	if(dir_map.find(string(filename))!= dir_map.end())
	{
		printf("Create File Error : File already exist.\n");
		return -1;
	}
	//check for free inode(if present i.e sb.next_freeinode != -1 )=in_free & set it 0 (0 means now this inode no is nt free)
	if(freeInodeVec.size()==0) //sb.next_freeinode == -1
	{
			printf("Create File Error : No more files can be created.Max number of files reached.\n");
			return -1;
	}

	//set pointer of sb.next_freeinode to point nextfree inode if no free inode available then set sb.next_freeinode=-1
	//check for free db(if present i.e != -1)
	if(freeDBvec.size() == 0) //sb.next_freedatablock == -1
	{
		printf("Create File Error : Not enough space.\n");
		return -1;
	}
	//assign a 4kb db & set
	nextin = freeInodeVec.back();
	freeInodeVec.pop_back();
	nextdb = freeDBvec.back();
	freeDBvec.pop_back();
	inode_arr[nextin].pointer[0]=nextdb;

	//set inode_arr[in_free].filesize=0;
	inode_arr[nextin].filesize = 0; // or 4KB whatever.
	//set value of map dir_map[string(filename)] = in_free;
	dir_map[string(filename)] = nextin;
	printf("entry created in dir_map for filename = %s & inode no assigned =%d\n",filename, nextin );
	//set directory structure also i.e strcpy(dir_structure[in_free].file_name, filename) & dir_structure[in_free].inode_num = in_free
	strcpy(dir_structure[nextin].file_name,filename);
	dir_structure[nextin].inode_num = nextin;
	//set current nextfreeinode and nextfreedb indices and choose nextfreeinode and nextfreedb indices

	//  no need to do this we'll use only freevectors now onwards
	//   	sb.inode_freelist[nextin]=1; // now this inode is reserved.
	//      sb.datablock_freelist[nextdb]= 1; // now this db is reserved.

	/*	for( i=0; i<NO_OF_INODES;i++)
	{
	if(sb.inode_freelist[i]==0)
		break;
	}
	if(i != NO_OF_INODES)
		next_freeinode = i;
	else
		next_freeinode = -1;

	for(i=sb.db_starting_index ; i < DISK_BLOCKS ; i++)
	{
	if(sb.datablock_freelist[i] == 0)
	break;
	}
	if(i != DISK_BLOCKS)
	next_freedatablock = i;
	else
	next_freedatablock = -1;
	*/
	printf(" %s file is created successfully.\n",filename );
	return 0;
}

int open_file(char* name) //assign fd to given filename & set cur pos to 0 and return fd
{
	//form a gloabl var openfile_count=0 & map with file descriptor as key and a pair of Inode and current cursor position as value
	//(coz  fd is key and unique fd can be assigned to one inodeNo)
	//each time on opening a file do openfile_count++ and get a free file descriptor from freeFileDescriptorsVec and make it as key and assign it to Inode
	// and on closing do openfile_count--
	//and remove the entry from map and push file descriptor in freeFileDescriptorsVec limit the count_of_openFile to 32 so that on exceeding openfile_count = 31 throw an error.
	// Can allocate the file descriptor until freeFileDescriptorsVec becomes empty.
	//allocate the file descriptor and set cursor position to 0 return the allocated file descriptor to the called function.

	int inode_no;
	if ( dir_map.find(string(name)) == dir_map.end() )
	{
  		printf("Open File Error : File Not Found\n");
  		return -1;
	}
 	else
 	{
 		//int file_des = getFreeFileDescriptor();
 		if(freeFileDescriptorsVec.size() == 0)
 		{
 			printf("Open File Error : No file descriptors available\n");
 			return -1;
 		}
 		else
 		{
 			int inode = dir_map[string(name)];
 			int fd = freeFileDescriptorsVec.back();
 			freeFileDescriptorsVec.pop_back();
 			fileDescriptor_map[fd].first=inode;
 			fileDescriptor_map[fd].second = 0;
 			openfile_count++;
 			//char inode_buf[sizeof(ino)]
 			/*for(int i=0;i<inode_arr[inode].filesize/BLOCK_SIZE;i++){

 			}*/
 			printf("Given %d as File descriptor\n",fd);

			//For Debugging Purpose Will remove later
 			/*printf("%d\n",inode_arr[inode].filesize );
 			printf("%d\n",inode_arr[inode].inode_num );
 			for (int i = 0; i < 13; ++i)
 			{
 				/* code
 				printf("%d\n",inode_arr[inode].pointer[i] );
 			}*/

 			return fd;

 		}
	}
	return -1;
}

int close_file(int fd)
{
	if ( fileDescriptor_map.find(fd) == fileDescriptor_map.end() )
	{
  		printf("File Descripter %d not found\n",fd);
  		return -1;
	}
 	else
 	{
		fileDescriptor_map.erase(fd);
		openfile_count--;
		freeFileDescriptorsVec.push_back(fd);
		printf("FD %d closed successfully\n",fd);

		return fd;
	}
	return -1;
}


int read_file(int fildes, char *buf, int nbyte)  //read the file specified by fd (fildes) starting from cur pos till EOF
{

	char dest_filename[20];
	int flag=0;
	if(fileDescriptor_map.find(fildes) == fileDescriptor_map.end()){
		printf("File Read Error: File not opened \n");
		return -1;
	}
	else
	{
		int inode =fileDescriptor_map[fildes].first;
		int noOfBlocks=ceil(((float)inode_arr[inode].filesize)/BLOCK_SIZE);
		int tot_block=noOfBlocks;
		strcpy(dest_filename, dir_structure[inode].file_name);


		printf("fd=%d inode block of reading file %d \n& cur_pos=%d\n",fildes,inode,fileDescriptor_map[fildes].second);
		printf("filesize %d noOfBlocks=%d dest_filename=%s\n",inode_arr[inode].filesize,noOfBlocks,dest_filename );

		FILE* fp1 = fopen(dest_filename,"wb+");
		char read_buf[BLOCK_SIZE];
		//noOfBlocks--;				//Why?  To leave last block unread using traditional while() because data in last block may be partially filled.

		for(int i=0;i<10;i++)
		{		//Reading data present at direct pointers
			if(noOfBlocks == 0){
				//printf("===>pointer[%d]=%d\n",i,inode_arr[inode].pointer[i] );
				break;

			}
			int block=inode_arr[inode].pointer[i];
			printf("===>pointer[%d]=%d\n",i,block );



			read_block(block,read_buf);


			// if(noOfBlocks > 1)
			// 	fwrite(read_buf,1,BLOCK_SIZE,fp1);


			if((tot_block-noOfBlocks >= FS/BLOCK_SIZE) && (noOfBlocks > 1))
			{
				if(flag==0)
				{
					fwrite(read_buf+(FS%BLOCK_SIZE),1,(BLOCK_SIZE-FS%BLOCK_SIZE),fp1);
					flag=1;
				}
				else
					fwrite(read_buf,1,BLOCK_SIZE,fp1);

			}


			noOfBlocks--;
		}

		if(noOfBlocks){		//Just to check any single indirect pointers are used or not
			int block = inode_arr[inode].pointer[10];
			//printf("==>pointer[10]=%d\n",block );
			int blockPointers[1024];		//Contains the array of data block pointers.
			read_block(block,read_buf);
			memcpy(blockPointers,read_buf,sizeof(read_buf));
			int i=0;
			while(noOfBlocks && i<1024){	//Should fail if all blocks are read or indirect pointers are done lets go for doble indirect
				printf("==>pointer[10]=%d-->blockpointer[%d]=%d\n",block,i,blockPointers[i]);
				//printf("==>pointer[10]=%d-->blockpointer[%d]=%d\n",block,i+1,blockPointers[i+1]);
				read_block(blockPointers[i++],read_buf);


			// if(noOfBlocks > 1)
			// 	fwrite(read_buf,1,BLOCK_SIZE,fp1);

			if(tot_block-noOfBlocks >= FS/BLOCK_SIZE && noOfBlocks > 1)
			{
				if(flag==0)
				{
					fwrite(read_buf+(FS%BLOCK_SIZE),1,(BLOCK_SIZE-FS%BLOCK_SIZE),fp1);
					flag=1;
				}
				else
					fwrite(read_buf,1,BLOCK_SIZE,fp1);

			}


			noOfBlocks--;
			}

		}
		if(noOfBlocks){		//Indirect pointers done check for Double Indirect
			int block = inode_arr[inode].pointer[11];
			int indirectPointers[1024];		//Contains array of indirect pointers
			read_block(block,read_buf);
			memcpy(indirectPointers,read_buf,sizeof(read_buf));
			int i=0;
			while(noOfBlocks && i<1024){
				read_block(indirectPointers[i++],read_buf);
				int blockPointers[1024];
				memcpy(blockPointers,read_buf,sizeof(read_buf));
				int j=0;
				while(noOfBlocks && j<1024)
				{
					printf("==>pointer[11]=%d-->indirectPointers[%d]=%d-->blockpointer[%d]=%d\n",block,i-1,indirectPointers[i-1],j,blockPointers[j] );
					read_block(blockPointers[j++],read_buf);
					// if(noOfBlocks > 1)
					// 	fwrite(read_buf,1,BLOCK_SIZE,fp1);

					if(tot_block-noOfBlocks >= FS/BLOCK_SIZE && noOfBlocks > 1)
					{
						if(flag==0)
						{
							fwrite(read_buf+(FS%BLOCK_SIZE),1,(BLOCK_SIZE-FS%BLOCK_SIZE),fp1);
							flag=1;
						}
						else
							fwrite(read_buf,1,BLOCK_SIZE,fp1);

					}




					noOfBlocks--;
				}
			}
		}

		if(tot_block-FS/BLOCK_SIZE > 1)
			fwrite(read_buf,1,(inode_arr[inode].filesize)%BLOCK_SIZE,fp1);
		else if(tot_block-FS/BLOCK_SIZE == 1)
			fwrite(read_buf+(FS%BLOCK_SIZE),1,(inode_arr[inode].filesize)%BLOCK_SIZE - FS%BLOCK_SIZE,fp1);


		fclose(fp1);

	}
}

int close_file()
{
	//remove the corresponding entry from above mentioned array=inode_to_fd [mentioned in open_file()]
}

int delete_file(char * filename)
{
	//if file is open show the prompt to user whether they want to continue delete (Y/N)
	//if filename doesn't exists then throw err
	//get inode no=in_num from map dir_map, go to that inode and track all DB and set sb.datablock_freelist[] = 0;//0 means DB is free
	//set  inode_arr[in_num].pointer[0-12] = -1  and set sb.inode_freelist[in_num]=0; //0 means inode is free
	//remove entry of this inode from map dir_map and directory st dir_structure
	//print and return the success msg
	char del_choice,ch;
	char abc[4096],def[4096];//for single and double;


	int indirect_arr[1024],indirectdb,double_indirect;
	bool flag_delete_finish =0; //to check if deletion is completed

	if(dir_map.count(string(filename)) == 0) // dir_map.find(filename) == dir_map.end())
	{
		printf("Delete File Error : File not exists.\n");
		return -1;
	}
	/*cout << string(filename) << " " << filename << endl;*/

	int inode = dir_map[string(filename)];
	/*printf("%d is inode num\n",inode );*/
	int count=0;
	for(int i=0;i < NO_OF_FILEDESCRIPTORS;i++)

	{
		if( (fileDescriptor_map.find(i) != fileDescriptor_map.end()) && (fileDescriptor_map[i].first == inode) ) //checking if file is open
		{

			printf("Delete File Error : File is opened cannot be deleted\n");
			return -1;
		}
			/*while( ( ch=getchar() )!='\n' );
			scanf("%c",&del_choice);

			printf("Your choice is  '%c'\n",del_choice );
			if(del_choice == 'N' || del_choice == 'n')
			{
				return -1;
			}
			else
			{
				int k=0;
				for (int j = NO_OF_FILEDESCRIPTORS-1; j >=i ; --j)
				{
					if(fileDescriptor_map.find(j)!=fileDescriptor_map.end() &&fileDescriptor_map[j].first == inode)
					{
						k++;
						freeFileDescriptorsVec.push_back(j);
						fileDescriptor_map.erase(j);

					}
				}
				printf("File is opened %d times\n",k );
				break;

			}*/

			//while( ( ch=getchar() )!='\n' );
	}

	//iterating through inode pointers [0-9][11][12]

	for (int i = 0; i < 10; ++i)
	{
		if(inode_arr[inode].pointer[i]==-1)
		{
			flag_delete_finish = true;
			break;
		}
		else
		{
			freeDBvec.push_back(inode_arr[inode].pointer[i]);
			inode_arr[inode].pointer[i]=-1;
		}
	}

	if(!flag_delete_finish)
	{
		 indirectdb = inode_arr[inode].pointer[10];
		 //char abc[4096];
		 //int indirect_arr[1024];
		 read_block(indirectdb,abc);
		 memcpy(indirect_arr,abc,sizeof(indirect_arr));
		 //now check for all 1024 add.
		 for(int i=0;i<1024;i++)
		 {
		 	if(indirect_arr[i]==-1)
		 	{
		 		flag_delete_finish = true;
				break;
		 	}
		 	else
		 	{
		 		freeDBvec.push_back(indirect_arr[i]);
			//inode_arr[inode].pointer[i]=-1;
		 	}
		 }


	}

	freeDBvec.push_back(indirectdb);   //push indeirectdb i.e. inode_arr[inode].pointer[10];
	inode_arr[inode].pointer[10]=-1; //now indirect add. block is set free;

	if(!flag_delete_finish)
	{


		 double_indirect = inode_arr[inode].pointer[11];
		int double_indirect_arr[1024];
		read_block(double_indirect,abc);
		memcpy(double_indirect_arr,abc,sizeof(double_indirect_arr));

		for(int i=0;i<1024;i++)
		{
			if(double_indirect_arr[i]==-1)
		 	{
		 		flag_delete_finish = true;
				break;
		 	}
		 	else
		 	{
		 		int single_indirect = double_indirect_arr[i];
		 		//delete_single(single_indirect)
		 		read_block(single_indirect,def);
		 		memcpy(indirect_arr,def,sizeof(indirect_arr));

		 		for(int j=0;j<1024 ;j++)
		 		{

		 			if(indirect_arr[j]==-1)
		 			{
		 				flag_delete_finish = true;
						break;
		 			}
		 			else
		 			{
		 				freeDBvec.push_back(indirect_arr[j]);
						//inode_arr[inode].pointer[i]=-1;
		 			}

		 		}

		 		//free this singlr_indirect block;
		 		freeDBvec.push_back(single_indirect);

		 	}
		}

		freeDBvec.push_back(double_indirect);

	}


	// now free the inode;

	for (int i = 0; i <= 12; ++i)
	{
		inode_arr[inode].pointer[i] = -1;
	}
	//Resetting inode structure with default values.
	inode_arr[inode].filesize = 0;
	inode_arr[inode].type = -1;
	inode_arr[inode].inode_num = -1;

	freeInodeVec.push_back(inode);
	char a[20]="";
	strcpy(dir_structure[inode].file_name,a);

	dir_structure[inode].inode_num=-1;
	dir_map.erase(filename);
	printf("\033[1;36mFile is deleted successfully.\033[0m\n");
	return 0;

}




int write_file(int fd, char * buf, int size) //writes 'size' no of bytes from buf into file pointed by fd from cur_pos mentioned in fd
{
	//take user filename input in global var filename[20] (already declared) and size of file in a local var write_file_size (why local coz it'll get updated in inode at the end of function)
	//check if avail DB size > write_file_size else throw error & return


	int cur_pos=fileDescriptor_map[fd].second, db_to_write;

	if( cur_pos%BLOCK_SIZE == 0 && size > BLOCK_SIZE ) //if cur_pos is at the end(oe begin) of DB i.e new byte will come in at 0th position of DB but size is > BLOCK_SIZE
	{
		printf("size of data to write is more than 1 DB size\n");
		return -1;
	}
	else if( (BLOCK_SIZE - cur_pos%BLOCK_SIZE) < size ) //if DB partially filled and remaing empty size is less than size to write in the block
	{
		printf("size of data to write is more than remainig empty size of DB\n");
	}

	int no_of_db_used_till_cur_pos = cur_pos / BLOCK_SIZE ;  //no_of_db_used_till_cur_pos stores the no of DB that is fully occupied by existing file

	if(cur_pos%BLOCK_SIZE != 0)  //if last DB is partially filled then fill remaining part of this DB
	{
		if(no_of_db_used_till_cur_pos < 10 )
		{
			db_to_write  = inode_arr[fileDescriptor_map[fd].first].pointer[no_of_db_used_till_cur_pos];
			// fseek(fp, (db_to_write * BLOCK_SIZE) + ( cur_pos%BLOCK_SIZE ), SEEK_SET);  //move the cur after last byte
			// fwrite(buf,1,size,fp); //write the buf into file

			write_block(db_to_write, buf, size, cur_pos%BLOCK_SIZE );

			if( inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos < size )  //updating the filesiz
				inode_arr[fileDescriptor_map[fd].first].filesize += size - (inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos);
			fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
		}
		else if(no_of_db_used_till_cur_pos < 1034 ) //i.e if last DB is at single indirect
		{
			int block = inode_arr[fileDescriptor_map[fd].first].pointer[10];
			int blockPointers[1024];		//Contains the array of data block pointers.
			char read_buf[4096];
			read_block(block,read_buf);  //reading the block into read_buf
			memcpy(blockPointers,read_buf,sizeof(read_buf));



			db_to_write  = blockPointers[no_of_db_used_till_cur_pos-10];
			// fseek(fp, (db_to_write * BLOCK_SIZE) + ( cur_pos%BLOCK_SIZE ), SEEK_SET);  //move the cur after last byte
			// fwrite(buf,1,size,fp); //write the buf into file

			write_block(db_to_write, buf, size, cur_pos%BLOCK_SIZE );

			if( inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos < size )  //updating the filesize
				inode_arr[fileDescriptor_map[fd].first].filesize += size - (inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos);
			fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
		}
		else
		{

			int block = inode_arr[fileDescriptor_map[fd].first].pointer[10];
			int blockPointers[1024];		//Contains the array of data block pointers.
			char read_buf[4096];
			read_block(block,read_buf);  //reading the block into read_buf
			memcpy(blockPointers,read_buf,sizeof(read_buf));

			int block2 = blockPointers[(no_of_db_used_till_cur_pos-1034)/1024];
			int blockPointers2[1024];		//Contains the array of data block pointers.
			read_block(block2,read_buf);  //reading the block2 into read_buf
			memcpy(blockPointers2,read_buf,sizeof(read_buf));



			db_to_write  = blockPointers2[(no_of_db_used_till_cur_pos-1034)%1024];
			// fseek(fp, (db_to_write * BLOCK_SIZE) + ( cur_pos%BLOCK_SIZE ), SEEK_SET);  //move the cur after last byte
			// fwrite(buf,1,size,fp); //write the buf into file
			write_block(db_to_write, buf, size, cur_pos%BLOCK_SIZE );

			if( inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos < size )  //updating the filesize
				inode_arr[fileDescriptor_map[fd].first].filesize += size - (inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos);
			fileDescriptor_map[fd].second += size;  //updating the cur pos in the file

		}
	}
	else //if if last DB is fully filled (or fully empty if filesize is 0) then fill full block of this DB
	{
		if(inode_arr[fileDescriptor_map[fd].first].filesize == 0) //if filesize = 0 means 1 empty DB is already assigned at pointer[0] by create_file()
		{
			write_block(inode_arr[fileDescriptor_map[fd].first].pointer[0], buf, size, 0 );
			inode_arr[fileDescriptor_map[fd].first].filesize += size; //updating filesize
			fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
		}
		else //if filesize is not 0
		{
			if(cur_pos<inode_arr[fileDescriptor_map[fd].first].filesize) //if last blok is partially filled & cur_pos at begin then fill this blok (no need to create new block)
			{
				if(no_of_db_used_till_cur_pos < 10 )
				{
					db_to_write  = inode_arr[fileDescriptor_map[fd].first].pointer[no_of_db_used_till_cur_pos];
					// fseek(fp, (db_to_write * BLOCK_SIZE) + ( cur_pos%BLOCK_SIZE ), SEEK_SET);  //move the cur after last byte
					// fwrite(buf,1,size,fp); //write the buf into file

					write_block(db_to_write, buf, size, 0 );

					if( inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos < size )  //updating the filesiz
						inode_arr[fileDescriptor_map[fd].first].filesize += size - (inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos);
					fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
				}
				else if(no_of_db_used_till_cur_pos < 1034 ) //i.e if last DB is at single indirect
				{
					int block = inode_arr[fileDescriptor_map[fd].first].pointer[10];
					int blockPointers[1024];		//Contains the array of data block pointers.
					char read_buf[4096];
					read_block(block,read_buf);  //reading the block into read_buf
					memcpy(blockPointers,read_buf,sizeof(read_buf));



					db_to_write  = blockPointers[no_of_db_used_till_cur_pos-10];
					// fseek(fp, (db_to_write * BLOCK_SIZE) + ( cur_pos%BLOCK_SIZE ), SEEK_SET);  //move the cur after last byte
					// fwrite(buf,1,size,fp); //write the buf into file

					write_block(db_to_write, buf, size, 0);

					if( inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos < size )  //updating the filesize
						inode_arr[fileDescriptor_map[fd].first].filesize += size - (inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos);
					fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
				}
				else
				{

					int block = inode_arr[fileDescriptor_map[fd].first].pointer[10];
					int blockPointers[1024];		//Contains the array of data block pointers.
					char read_buf[4096];
					read_block(block,read_buf);  //reading the block into read_buf
					memcpy(blockPointers,read_buf,sizeof(read_buf));

					int block2 = blockPointers[(no_of_db_used_till_cur_pos-1034)/1024];
					int blockPointers2[1024];		//Contains the array of data block pointers.
					read_block(block2,read_buf);  //reading the block2 into read_buf
					memcpy(blockPointers2,read_buf,sizeof(read_buf));



					db_to_write  = blockPointers2[(no_of_db_used_till_cur_pos-1034)%1024];
					// fseek(fp, (db_to_write * BLOCK_SIZE) + ( cur_pos%BLOCK_SIZE ), SEEK_SET);  //move the cur after last byte
					// fwrite(buf,1,size,fp); //write the buf into file
					write_block(db_to_write, buf, size, 0 );

					if( inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos < size )  //updating the filesize
						inode_arr[fileDescriptor_map[fd].first].filesize += size - (inode_arr[fileDescriptor_map[fd].first].filesize - cur_pos);
					fileDescriptor_map[fd].second += size;  //updating the cur pos in the file

				}
			}
			else //if cur pos == filesze and its end of block i.e muliple of 4096
			{
				if(no_of_db_used_till_cur_pos < 10 )
				{
					db_to_write  =  freeDBvec.back();
					freeDBvec.pop_back();
					inode_arr[fileDescriptor_map[fd].first].pointer[no_of_db_used_till_cur_pos] = db_to_write;
					write_block(db_to_write, buf, size, 0 );

					inode_arr[fileDescriptor_map[fd].first].filesize += size;
					fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
				}
				else if(no_of_db_used_till_cur_pos < 1034 ) //i.e if last DB is at last of single indirect DB or at last of pointer[9]
				{
					if(no_of_db_used_till_cur_pos == 10) //i.e all direct DB pointer 0-9 is full then need to allocate a DB to pointer[10] & store 1024 DB in this DB
					{
						int blockPointers[1024];
						for (int i = 0; i < 1024; ++i)
							blockPointers[i] = -1;

						int db_for_single_indirect = freeDBvec.back();  //to store blockPointers[1024] into db_for_single_indirect
						freeDBvec.pop_back();

						inode_arr[fileDescriptor_map[fd].first].pointer[10]=db_for_single_indirect;
						char temp_buf[4096];
						memcpy(temp_buf,blockPointers,BLOCK_SIZE);

						write_block(db_for_single_indirect, temp_buf, BLOCK_SIZE, 0 );

					}

					int block = inode_arr[fileDescriptor_map[fd].first].pointer[10];
					int blockPointers[1024];		//Contains the array of data block pointers.
					char read_buf[4096];
					read_block(block,read_buf);  //reading the single indirect block into read_buf
					memcpy(blockPointers,read_buf,sizeof(read_buf)); //moving the data into blockPointer

					db_to_write  =  freeDBvec.back();
					freeDBvec.pop_back();

					//store back the blockPointer to disk
					blockPointers[no_of_db_used_till_cur_pos-10] = db_to_write;
					char temp_buf[4096];
					memcpy(temp_buf,blockPointers,BLOCK_SIZE);
					write_block(block, temp_buf, BLOCK_SIZE, 0 );

					//write data into DB
					write_block(db_to_write, buf, size, 0);

					inode_arr[fileDescriptor_map[fd].first].filesize += size;
					fileDescriptor_map[fd].second += size;  //updating the cur pos in the file
				}
				else
				{
					if(no_of_db_used_till_cur_pos == 1034) //if all direct and single direct is full then assign a DB to double indirect all init to -1
					{
						int blockPointers[1024];
						for (int i = 0; i < 1024; ++i)
							blockPointers[i] = -1;

						int db_for_double_indirect = freeDBvec.back();  //to store blockPointers[1024] into db_for_double_indirect
						freeDBvec.pop_back();

						inode_arr[fileDescriptor_map[fd].first].pointer[11]=db_for_double_indirect;
						char temp_buf[4096];
						memcpy(temp_buf,blockPointers,BLOCK_SIZE);

						write_block(db_for_double_indirect, temp_buf, BLOCK_SIZE, 0 );
					}
					if( (no_of_db_used_till_cur_pos-1034)%1024==0 ) //i.e if no_of_db_used_till_cur_pos is multiple of 1024 means need new DB to be assigned
					{
						int block = inode_arr[fileDescriptor_map[fd].first].pointer[11];
						int blockPointers[1024];		//Contains the array of data block pointers.
						char read_buf[4096];
						read_block(block,read_buf);  //reading the block into read_buf
						memcpy(blockPointers,read_buf,sizeof(read_buf));



						int blockPointers2[1024];
						for (int i = 0; i < 1024; ++i)
							blockPointers2[i] = -1;

						int db_for_double_indirect2 = freeDBvec.back();  //to store blockPointers[1024] into db_for_double_indirect
						freeDBvec.pop_back();

						blockPointers[(no_of_db_used_till_cur_pos-1034)/1024]=db_for_double_indirect2;
						char temp_buf[4096];
						memcpy(temp_buf,blockPointers2,BLOCK_SIZE);
						write_block(db_for_double_indirect2, temp_buf, BLOCK_SIZE, 0 );

						memcpy(temp_buf,blockPointers,BLOCK_SIZE);
						write_block(block, temp_buf, BLOCK_SIZE, 0 );

					}


					int block = inode_arr[fileDescriptor_map[fd].first].pointer[11];
					int blockPointers[1024];		//Contains the array of data block pointers.
					char read_buf[4096];
					read_block(block,read_buf);  //reading the block into read_buf
					memcpy(blockPointers,read_buf,BLOCK_SIZE);

					//#####################################################


					int block2 = blockPointers[(no_of_db_used_till_cur_pos-1034)/1024];
					int blockPointers2[1024];		//Contains the array of data block pointers.
					char read_buf2[4096];
					read_block(block2,read_buf2);  //reading the block2 into read_buf2
					memcpy(blockPointers2,read_buf2,BLOCK_SIZE);


					int db_to_write = freeDBvec.back();  //to store blockPointers[1024] into db_for_double_indirect
					freeDBvec.pop_back();
					blockPointers2[(no_of_db_used_till_cur_pos-1034)%1024]=db_to_write;
					write_block(db_to_write, buf, size, 0 );  //writing data into db_to_write DB

					//now restore blockPointers2 back to the block2
					char temp_buf[4096];
					memcpy(temp_buf,blockPointers2,BLOCK_SIZE);
					write_block(block2, temp_buf, BLOCK_SIZE, 0 );


					//updating the filesize
					inode_arr[fileDescriptor_map[fd].first].filesize += size;
					fileDescriptor_map[fd].second += size;  //updating the cur pos in the file

				}

			}
		}
	}

}

int store_file_into_Disk(char * filename)
{

	int fp2_filesize, fd2, size_buf2, no_of_db_in_src_file, remain_size_OfLast_db;
	char buf2[4097];
	FILE *fp2;
	fp2=fopen(filename,"rb");

	if(fp2 == NULL)
	{
		printf("Source File doen not exists\n");
		return -1;
	}

	fseek(fp2, 0L, SEEK_END); // file pointer at end of file
    fp2_filesize = ftell(fp2);  //getting the position
    //no_of_db_in_src_file = fp2_filesize / BLOCK_SIZE;
    fseek( fp2, 0, SEEK_SET );

		if( (freeDBvec.size() * 4096) < fp2_filesize )
		{
			printf("Not Enough space in Disk\n");
			return -1;
		}
		char* filename1 =strrchr(filename,'/');
		if(filename1 == NULL){
			filename1 = filename;
		}
		else{
			filename1 = filename1+1;
		}
	if( create_file(filename1) < 0 )
	{
		printf("Error in create_file()\n");
		return -1;
	}
	if( (fd2=open_file(filename1)) < 0)
	{
		printf("Error in open_file()\n");
		return -1;
	}




	remain_size_OfLast_db = BLOCK_SIZE - ( (fileDescriptor_map[fd2].second) % BLOCK_SIZE ); //checking how many in last DB have space remaing to store data

	if( remain_size_OfLast_db >= fp2_filesize ) //if remaing empty size is more than src file size then write directly in last DB
	{
		fread(buf2, fp2_filesize, 1, fp2);
		buf2[fp2_filesize] = '\0';
		write_file(fd2, buf2, fp2_filesize);
	}
	else
	{
		printf("filesize before writing anything = %d\n", inode_arr[fileDescriptor_map[fd2].first].filesize );
		//1st write remain_size_OfLast_db
		fread(buf2, remain_size_OfLast_db, 1, fp2);
		buf2[remain_size_OfLast_db] = '\0';
		write_file(fd2, buf2, remain_size_OfLast_db);
		//printf("1st : remain_size_OfLast_db = %d written now filesize = %d\n",remain_size_OfLast_db,inode_arr[fileDescriptor_map[fd2].first].filesize );
		//write block by block till last block
		int remaining_block_count = (fp2_filesize - remain_size_OfLast_db) / BLOCK_SIZE;
		while(remaining_block_count--)
		{
			fread(buf2, BLOCK_SIZE, 1, fp2);
			buf2[BLOCK_SIZE] = '\0';
			write_file(fd2, buf2, BLOCK_SIZE);
			//printf("Written 1 block now remaining_block_count = %d now filesize = %d\n",remaining_block_count-1,inode_arr[fileDescriptor_map[fd2].first].filesize );
		}
		//write last partial block
		int remaining_size = (fp2_filesize - remain_size_OfLast_db) % BLOCK_SIZE;

		fread(buf2, remaining_size, 1, fp2);
		buf2[remaining_size] = '\0';
		write_file(fd2, buf2, remaining_size);
		//printf("Remaing size = %d written = %s now filesize=%d\n",remaining_size,buf2,inode_arr[fileDescriptor_map[fd2].first].filesize );

	}

	close_file(fd2);



	return 0;
}


//Helper function
int read_block(int block,char* buf)
{
	if ((block < 0) || (block >= DISK_BLOCKS)) {
    printf("Block read Error : block index out of bounds\n");
    return -1;
  }

  if (fseek(fp, block * BLOCK_SIZE, SEEK_SET) < 0) {
    printf("Block read Error : failed to lseek");
    return -1;
  }
  if (fread(buf, 1, BLOCK_SIZE,fp) < 0) {
    printf("Block read Error : failed to read");
    return -1;
  }
  return 0;
}

//Helper function
int write_block(int block, char *buf, int size_to_wrt, int start_pos_to_wrt) //in gen write_block(5, buf, 4096, 0) i.e write all data(of 4096 size) into DB no=5 starting from 0th pos
{																			 // 0 < size_to_wrt <= 4096  & 0 <= start_pos_to_wrt <= 4095

  if ((block < 0) || (block >= DISK_BLOCKS))
  {
    printf("Block write Error : block index out of bounds\n");
    return -1;
  }

  if (fseek(fp, (block * BLOCK_SIZE)+start_pos_to_wrt, SEEK_SET) < 0)
  {
    printf("Block write Error : failed to lseek");
    return -1;
  }

  if (fwrite(buf, 1, size_to_wrt,fp) < 0)
  {
    perror("block_write: failed to write");
    return -1;
  }

  return 0;
}


//--------------------------------------------------------------------------------
int main()
{
	user_inputs();






	//unmounting();
	return 0;
}
//--------------------------------------------------------------------------------
