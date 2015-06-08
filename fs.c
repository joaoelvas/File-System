// This is an Academic Project, and was published after finishing the lecture.
// @author Joao Elvas @ FCT/UNL

#include "fs.h"
#include "disk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <strings.h>

#define FALSE 0
#define TRUE 1

#define SUPERBLOCK 0
#define DIRECTORY 1
#define FAT 2

#define SIZE 1024

//super block
#define FS_MAGIC           0xf0f03410
typedef struct{
	int magic;
	int nblocks;
	int nfatblocks;
	char filler[DISK_BLOCK_SIZE-3*sizeof(int)];
} super_block;

super_block mb;

//directory
#define MAX_NAME_LEN 6
#define VALID 1
#define NON_VALID 0
typedef struct{
	unsigned char used;
	char name[MAX_NAME_LEN+1];
	unsigned int length;
	unsigned int first_block;
} dir_entry;

#define INUSE 1
#define N_DIR_ENTRIES (DISK_BLOCK_SIZE / sizeof(dir_entry))
dir_entry dir[N_DIR_ENTRIES];

// file allocation table
#define N_ADDRESSES_PER_BLOCK (DISK_BLOCK_SIZE / sizeof(int))
#define FREE 0
#define BUSY 2
#define EOFF 1
unsigned int *fat;

#define NOTMOUNTED 0
#define MOUNTED 1
int mountState = 0;

/* Creates a new file system in the current disk, wiping out all the data.
 * Block 0 will contain the magic block, Block 1 is used for directory; 
 * all directory entries are marked as free. The file allocation table starts at 
 * block 2 and occupies ( nblocks / 1024) disk blocks rounded to the next integer. 
 * If one tries to format a mounted disk, fs_format should do nothing and return an error. 
 * Returns 0 in case of success and -1 if an error occurs.
 */
int fs_format(){                    //FUNKA

  disk_read(SUPERBLOCK, (char *)&mb);
  if(mb.magic == FS_MAGIC){
    printf("Refusing to format a formatted disk!\n");
    return -1;
}
  //FORMATAR SUPERBLOCO
  mb.magic = FS_MAGIC;              //ATRIBUI O MAGIC NUMBER DO FORMATO PRETENDIDO
  mb.nblocks = disk_size();         //NUMERO DE BLOCOS DO DISCO
  mb.nfatblocks = mb.nblocks/SIZE;  //NUMERO DE NFATBLOCKS

  if(mb.nblocks%SIZE >= 0)
    mb.nfatblocks++;

  disk_write(SUPERBLOCK, (char *)&mb);
 
  //FORMATAR DIRECTORIA
  int i, j;
  //for(i = 0; dir[i].used == 0; i++)
  for(i = 0; i < N_DIR_ENTRIES; i++)
  {
    dir[i].used = NON_VALID;
  }
  disk_write(DIRECTORY,(char*) &dir);

  //FORMATAR FAT
  fat = calloc(DISK_BLOCK_SIZE,mb.nfatblocks);
  fat[SUPERBLOCK] = BUSY;
  fat[DIRECTORY] = BUSY;

  for (i = FAT; i < FAT + mb.nfatblocks; i++)
  {
    fat[i] = BUSY;
  }

  for (j = FAT; j < FAT + mb.nfatblocks; j++) {
    disk_write(j,(char*) fat);
  }

  free(fat);
  
  return 0;
}

/* Displays information about the current active disk. The expected output is something like:
 *     superblock:
 *          magic number is valid
 *          1010 blocks on disk
 *          1 blocks for file allocation table
 *     File “xpto1”:
 *          size: 4500 bytes
 *          Blocks: 103 194
 *     File “hello.c”:
 *          size 11929 bytes
 *          Blocks: 105 109 210
 * This should work either the disk is mounted or not. If the disk does not contain a valid file 
 * system – indicated by the absence of the magic number in the beginning of the super block – the 
 * command should return immediately indicating why.
 */
void fs_debug()                               //FUNKA
{
  super_block tmpmb;
  disk_read(SUPERBLOCK, (char *)&tmpmb);

  dir_entry tmpdir[N_DIR_ENTRIES];
  disk_read(DIRECTORY,(char*) &tmpdir);


  unsigned int *tmpfat;
  tmpfat = calloc(DISK_BLOCK_SIZE,tmpmb.nfatblocks);

  int i,j;
  for (j = 0; j <  tmpmb.nfatblocks; j++) {
    disk_read(j+FAT,(char*) tmpfat + j * DISK_BLOCK_SIZE); 
  }

  printf("superblock:\n");

  if(tmpmb.magic == FS_MAGIC)
  {
    printf("     magic number is valid\n");
    printf("     %d blocks on disk\n", tmpmb.nblocks);
    printf("     %d blocks for file allocation table\n", tmpmb.nfatblocks);
    for(i = 0; i < N_DIR_ENTRIES; i++)
    {
      if(tmpdir[i].used == VALID)
      { 
        printf("File \"%s\" :\n", tmpdir[i].name);
        printf("     size: %d bytes\n", tmpdir[i].length);

        j = tmpdir[i].first_block;
        printf("     ");
        while(j != EOFF) {
          printf("%d ", j);
          j = tmpfat[j];
        }


        printf("\n");

      }
    }
  }
  else
    printf("  magic number isnt valid\n");

  free(tmpfat);
}

/* Verifies if there is a valid file system in the current disk. If there is one, reads the directory 
 * and the file allocation table to RAM. Note that all the following operations will fail if there is 
 * not a mounted disk. Returns 0 on success and -1 on fail.
 */
int fs_mount()                                //FUNKA
{
  int i;
  if(mountState == MOUNTED){                  //VERIFICA SE O DISCO JA ESTA MONTADO
    printf("disc already mounted!\n");
    return -1;
  }
  disk_read(SUPERBLOCK,(char*) &mb);          //LE OS VALORES DO SUPERBLOCO DO DISCO PARA A RAM

  if(mb.magic != FS_MAGIC) {
    printf("Disk not formated!\n");
    return -1;
  }

  disk_read(DIRECTORY,(char*) &dir);          //LE OS VALORES DA DIRETORIA PARA A RAM


  fat = calloc(mb.nblocks,N_ADDRESSES_PER_BLOCK);
  for (i = 0; i <  mb.nfatblocks; i++)                                //LE OS VALORES DA FAT PARA A RAM TANTAS ESCREVENDO EM TANTOS
  {
    disk_read(i + FAT,(char*) (fat + (i * N_ADDRESSES_PER_BLOCK)));   //BLOCOS QUANTO OS QUE A FAT OCUPA
  }

  mountState = MOUNTED;

  return 0;
}

/* Creates an entry on the directory describing an empty file (length 0) with name name; the directory
 * is updated in RAM and in the disk. Returns 0 on success and -1 on fail.
 */
int fs_create(char *name)                             //FUNKA MAS NAO FUNKA - NAO CRIA NADA
{
  int i = 0;

  if(mountState == NOTMOUNTED){
    printf("disc not mounted\n");
    return -1;
  }
  int found = FALSE;

  while (!found || i >= N_DIR_ENTRIES)
  {
    if(dir[i].used == FREE ) found = TRUE;
    else i++;
  }
  if(!found) return -1;
  dir[i].used = INUSE;
  strncpy(dir[i].name, name,MAX_NAME_LEN+1);
  dir[i].length = 0;

  dir[i].first_block = EOFF;

  disk_write(DIRECTORY, (char*) dir);

  return 0;
}

/* Removes the file name. Frees all the blocks associated with name and updates the file allocation 
 * table in RAM and in the disk; after this releases the directory entry. Returns 0 in case of success; 
 * -1 in case of error
 */
int fs_delete( char *name )                                       //FUNKA LIKE A BOSS
{
  int i = -1,d = -1,j = -1,x = -1;
  if(mountState == NOTMOUNTED){
    printf("disc not mounted\n");
    return -1;
  }
  // por a 0 na diretoria, ir a fat e por os blocos que o ficheiro ocupa a 0
  for (i = 0; i < N_DIR_ENTRIES; ++i) {
    if(strcmp(name,dir[i].name) == 0) {
      j = fat[dir[i].first_block];
      d = i;
    }
  }
  dir[d].used = FREE;
  for (int i = 0; i < DISK_BLOCK_SIZE * mb.nfatblocks; ++i)
  {
    while(j != EOFF && dir[d].length != 0 ) {
      x = j;
      j = fat[j];
      fat[x] = FREE;
      
    }
  }

  disk_write(DIRECTORY, (char*) dir);



  return 0;
}

/* Returns the length in bytes of the file associated with the specified name. In case of error returns -1.
 */
int fs_getsize( char *name ){                     //FUNKA

  int i, d;

  if(mountState == NOTMOUNTED){
    printf("disc not mounted\n");
    return -1;
  }

  for (i = 0; i < N_DIR_ENTRIES; ++i) {
    if(strcmp(name,dir[i].name) == 0) {
      d = i;
    }
  }

  return dir[d].length;
   
}

int iBlock(int i, int indice) {
  int j = 0;
  int b = dir[i].first_block;
  while(j < indice) {
    b = fat[b];
    j++;
  }
  return b;
}

/* Reads data from a valid file. Copies length bytes of the file specified by name to the address specified
 * by the pointer data, starting at offset bytes from the beginning of file. Returns the total number of
 * read bytes. This returned number can be less than the value specified in length if the end of file is reached.
 * In case of error returns -1.
 */
int fs_read( char *name, char *data, int length, int offset )
{

  if(mountState == NOTMOUNTED){
    printf("disc not mounted\n");
    return -1;
  }

  char buffer[DISK_BLOCK_SIZE];
  int fileSize = fs_getsize(name);
  int blocknr = -1, i, readBytes = 0;
  int bytestoread=0,indice_bloco=0,fim=0,offsetBloco=0,nb;


  for (i = 0; i < N_DIR_ENTRIES; ++i) {
    if(!strcmp(name,dir[i].name)) {
      break;
    }
  }

  if(i == N_DIR_ENTRIES) {
    printf("ficheiro inexistente\n");
    return -1;
  }

  if(offset > fileSize) {
    printf("offset inicial superior ao cumprimento do ficheiro\n");
    return -1;
  }

  if(offset + length > fileSize) {
    bytestoread = fileSize - offset;
  } else bytestoread = length;

  int offsetCurrent = offset;
  

  while(readBytes < bytestoread) {
    indice_bloco = offsetCurrent / DISK_BLOCK_SIZE;
    offsetBloco = offset % DISK_BLOCK_SIZE;
    blocknr = iBlock(i,indice_bloco);
    disk_read(blocknr, buffer);
    

    if(dir[i].length < (indice_bloco+1) * DISK_BLOCK_SIZE) {
      fim = dir[i].length - indice_bloco*DISK_BLOCK_SIZE;
    } else { 
      fim = DISK_BLOCK_SIZE;
    }

    nb = fim - offsetBloco;

    bcopy(buffer+offsetBloco, data+readBytes, nb);

    readBytes = readBytes + nb;
    offsetCurrent = offsetCurrent + nb;

}

return readBytes;

}



/* Writes data to a file. Copies length bytes from the memory address data to the file identified by name
 * starting in file offset offset. In general, this will imply the allocation of free blocks. The function returns
 * the effective number of bytes written, which can be smaller than the value specified in length, for example if
 * the disk becomes full. Returns -1 in case of error.
 */
int fs_write( char *name, const char *data, int length, int offset )
{
 

  if(mountState == NOTMOUNTED){
    printf("disc not mounted\n");
    return -1;
  }
  
  char buffer[DISK_BLOCK_SIZE];
  int fileSize = fs_getsize(name);
  int blocknr = -1, i, readBytes = 0;
  int bytestowrite=0,indice_bloco=0,fim=0,offsetBloco=0,nb;



  for (i = 0; i < N_DIR_ENTRIES; ++i) {
    if(!strcmp(name,dir[i].name)) {
      break;
    }
  }

  if(i == N_DIR_ENTRIES) {
    printf("ficheiro inexistente\n");
    return -1;
  }

  if(offset > fileSize) {
    printf("offset inicial superior ao cumprimento do ficheiro\n");
    return -1;
  }

  if(offset + length > fileSize) {
    bytestowrite = fileSize - offset;
  } else bytestowrite = length;

  int offsetCurrent = offset;
  int nrBlocoFicheiro = ceil(dir[i].length/DISK_BLOCK_SIZE);
  int nBytesWritten = 0;
  

  while(nBytesWritten < bytestowrite) {
    indice_bloco = offsetCurrent / DISK_BLOCK_SIZE;
    offsetBloco = offset % DISK_BLOCK_SIZE;

    if(indice_bloco < nrBlocoFicheiro - 1) {
      blocknr = iBlock(i,indice_bloco);
      disk_read(blocknr,buffer);
      bcopy(data+readBytes, buffer+offsetBloco, nb); //VER ESTA MERDA
      disk_write(blocknr,buffer);
    } else {
      bcopy(data+readBytes, buffer+offsetBloco, nb);
      disk_write(blocknr,buffer);
    }
    

    if(dir[i].length < (indice_bloco+1) * DISK_BLOCK_SIZE) {
      fim = dir[i].length - indice_bloco*DISK_BLOCK_SIZE;
    } else { 
      fim = DISK_BLOCK_SIZE;
    }

    nb = fim - offsetBloco;

    readBytes = readBytes + nb;
    offsetCurrent = offsetCurrent + nb;

}

  return readBytes;

}
