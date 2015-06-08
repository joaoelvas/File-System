# File-System
This programming assignment concerns the development of a file system similar to Windows FAT.

Assignment objectives
- To consolidate the knowledge about how a file system works.
- To add competences in the use of the C programming language.

This assignment can be solved in any environment where a C compiler is available.

#MIEI-02 file system1

This programming assignment concerns the development of a file system similar to Windows FAT; 
of course, many simplifications are made. Students will receive a set of files with code; one of 
the files is very incomplete and we ask you to fill the blanks in it. Students should not change 
the contents of the other files.
As expected the file system is stored in a disk; a Linux (or Windows or Mac OS X) file simulates 
the disk. Reading and writing blocks corresponds to reading and writing fixed size chunks of data 
from / to the file; all reads and writes start at an offset multiple of the block size.

#Disk emulation
The disk is emulated by a file; one can only read or write 4K bytes that start in a offset that
is a multiple of 4096. File disk.h defines the API for using the virtual disk:

#define DISK_BLOCK_SIZE 4096
int disk_init( const char *filename, int nblocks ); 
int disk_size();
void disk_read( int blocknum, char *data );
void disk_write( int blocknum, const char *data ); 
void disk_close();

#File system operations

File fs.h describes the operations that manipulate the file system:
int fs_format();
void fs_debug();
int fs_mount();
int fs_create( char *name);
int fs_delete( char *name );
int fs_getsize( char *name );
int fs_read( char *name, char *data, int length, int offset );
int fs_write( char *name, const char *data, int length, int offset );

In the following table the actions corresponding to each file system operation are described:

int  fs_format()
Creates a new file system in the current disk, wiping out all the data. Block 0 will contain
the magic block, Block 1 is used for directory; all directory entries are marked as free. 
The file allocation table starts at block 2 and occupies ( nblocks / 1024) disk blocks 
rounded to the next integer. If one tries to format a mounted disk, fs_format should do 
nothing and return an error. Returns 0 in case of success and -1 if an error occurs.


void fs_debug()
Displays information about the current active disk. The expected output is something like:
      superblock:
           magic number is valid
           1010 blocks on disk
           2 blocks for file allocation table
      File “xpto1”:
           size: 4500 bytes
           Blocks: 103 194
      File “hello.c”:
           size 11929 bytes
           Blocks: 105 109 210
This should work either the disk is mounted or not. If the disk does not contain a valid file 
system – indicated by the absence of the magic number in the beginning of the super block – 
the command should return immediately indicating why.

int  fs_mount()
Verifies if there is a valid file system in the current disk. If there is one, reads the directory
and the file allocation table to RAM. Note that all the following operations will fail if there is 
not a mounted disk. Returns 0 on success and -1 on fail.

int  fs_create( char *name)
Creates an entry on the directory describing an empty file (length 0) with name name; the directory
is updated in RAM and in the disk. Returns 0 on success and -1 on fail.

int  fs_delete( char *name )
Removes the file name. Frees all the blocks associated with name and updates the file allocation 
table in RAM and in the disk; after this releases the directory entry. Returns 0 in case of success;
-1 in case of error

int  fs_getsize( char *name )
Returns the length in bytes of the file associated with the specified name. In case of error returns -1.

int fs_read( char *name, char *data, int length, int offset )
Reads data from a valid file. Copies length bytes of the file specified by name to the address specified
by the pointer data, starting at offset bytes from the beginning of file. Returns the total number 
of read bytes. This returned number can be less than the value specified in length if the end of file 
is reached. In case of error returns -1.

int fs_write( char *name, const char *data, int length, int offset )
Writes data to a file. Copies length bytes from the memory address data to the file identified by name 
starting in file offset offset. In general, this will imply the allocation of free blocks. The function
returns the effective number of bytes written, which can be smaller than the value specified in length,
for example if the disk becomes full. Returns -1 in case of error.
