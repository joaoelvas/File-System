// This is an Academic Project, and was published after finishing the lecture.
// @author Joao Elvas @ FCT/UNL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "disk.h"


static FILE *diskfile;
static int nblocks=0;
static int nreads=0;
static int nwrites=0;

int disk_init( const char *filename, int n )
{
	diskfile = fopen(filename,"r+");
	if(!diskfile) diskfile = fopen(filename,"w+");
	if(!diskfile) return 0;

	ftruncate(fileno(diskfile),n*DISK_BLOCK_SIZE);

	nblocks = n;
	nreads = 0;
	nwrites = 0;

	return 1;
}

int disk_size()
{
	return nblocks;
}

static void sanity_check( int blocknum, const void *data )
{
	if(blocknum<0) {
		printf("ERROR: blocknum (%d) is negative!\n",blocknum);
		abort();
	}

	if(blocknum>=nblocks) {
		printf("ERROR: blocknum (%d) is too big!\n",blocknum);
		abort();
	}

	if(!data) {
		printf("ERROR: null data pointer!\n");
		abort();
	}
}

void disk_read( int blocknum, char *data )
{
	int r;
	sanity_check(blocknum,data);

	fseek(diskfile,blocknum*DISK_BLOCK_SIZE,SEEK_SET);

	r = fread(data,DISK_BLOCK_SIZE,1,diskfile);
	if(r==1) {
		nreads++;
	} else {
		printf("ERROR: couldn't access simulated disk\n");
		perror("disk_read");
		exit(1);
	}
}

void disk_write( int blocknum, const char *data )
{
	int r;
	sanity_check(blocknum,data);

	fseek(diskfile,blocknum*DISK_BLOCK_SIZE,SEEK_SET);

	r = fwrite(data,DISK_BLOCK_SIZE,1,diskfile);
	if(r==1) {
		nwrites++;
	} else {
		printf("ERROR: couldn't access simulated disk\n");
		perror("disk write");
		exit(1);
	}
}

void disk_close()
{
	printf("%d disk block reads\n",nreads);
	printf("%d disk block writes\n",nwrites);
	fclose(diskfile);
}

