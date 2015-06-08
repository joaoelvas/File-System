// This is an Academic Project, and was published after finishing the lecture.
// @author Joao Elvas @ FCT/UNL

#ifndef DISK_H
#define DISK_H

#define DISK_BLOCK_SIZE 4096

int  disk_init( const char *filename, int nblocks );
int  disk_size();
void disk_read( int blocknum, char *buffer );
void disk_write( int blocknum, const char *buffer );
void disk_close();


#endif
