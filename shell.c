// This is an Academic Project, and was published after finishing the lecture.
// @author Joao Elvas @ FCT/UNL

#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int do_copyin( char *filename, char * myfs_name);
int do_copyout( char * myfs_name,  char *filename );

int main( int argc, char *argv[] )
{
	char line[1024];
	char cmd[1024];
	char arg1[1024];
	char arg2[1024];
	int result, args;

	if(argc!=3) {
		printf("use: %s <diskfile> <nblocks>\n",argv[0]);
		return 1;
	}

	if(!disk_init(argv[1],atoi(argv[2]))) {
		printf("couldn't initialize %s: %s\n",argv[1],strerror(errno));
		return 1;
	}

	printf("opened emulated disk image %s with %d blocks\n",argv[1],disk_size());

	while(1) {
		printf(" prompt> ");
		fflush(stdout);

		if(!fgets(line,sizeof(line),stdin)) break;

		if(line[0]=='\n') continue;
		line[strlen(line)-1] = 0;

		args = sscanf(line,"%s %s %s",cmd,arg1,arg2);
		if(args==0) continue;

		if(!strcmp(cmd,"format")) {
			if(args==1) {
				if(!fs_format()) {
					printf("disk formatted.\n");
				} else {
					printf("format failed!\n");
				}
			} else {
				printf("use: format\n");
			}
		} else if(!strcmp(cmd,"mount")) {
			if(args==1) {
				if(!fs_mount()) {
					printf("disk mounted.\n");
				} else {
					printf("mount failed!\n");
				}
			} else {
				printf("use: mount\n");
			}
		} else if(!strcmp(cmd,"debug")) {
			if(args==1) {
				fs_debug();
			} else {
				printf("use: debug\n");
			}
		} else if(!strcmp(cmd,"getsize")) {
			if(args==2) {
				result = fs_getsize(arg1);
				if(result>=0) {
					printf("file %s has size %d\n",arg1,result);
				} else {
					printf("getsize failed!\n");
				}
			} else {
				printf("use: getsize <filename>\n");
			}
			
		} else if(!strcmp(cmd,"create")) {
			if(args==2) {
				result = fs_create(arg1);
				if(result==0) {
					printf("created file %s\n",arg1);
				} else {
					printf("create failed!\n");
				}
			} else {
				printf("use: create <filename>\n");
			}
		} else if(!strcmp(cmd,"delete")) {
			if(args==2) {
				if(!fs_delete(arg1)) {
					printf("file %s deleted.\n",arg1);
				} else {
					printf("delete failed!\n");	
				}
			} else {
				printf("use: delete <filename>\n");
			}
		} else if(!strcmp(cmd,"cat")) {
			if(args==2) {
				if(!do_copyout(arg1,"/dev/stdout")) {
					printf("cat failed!\n");
				}
			} else {
				printf("use: cat <name>\n");
			}

		} else if(!strcmp(cmd,"copyin")) {
			if(args==3) {
				if(do_copyin(arg1,arg2)) {
					printf("copied file %s to  %s\n",arg1,arg2);
				} else {
					printf("copy failed!\n");
				}
			} else {
				printf("use: copyin <filename in host system> <filename in fs-miei-02>\n");
			}

		} else if(!strcmp(cmd,"copyout")) {
			if(args==3) {
				if(do_copyout(arg1,arg2)) {
					printf("copied myfs_file %s to file %s\n", arg1,arg2);
				} else {
					printf("copy failed!\n");
				}
			} else {
				printf("use: copyout <inumber> <filename>\n");
			}

		} else if(!strcmp(cmd,"help")) {
			printf("Commands are:\n");
			printf("    format\n");
			printf("    mount\n");
			printf("    debug\n");
			printf("    create\n");
			printf("    delete  <filename>\n");
			printf("    cat     <filename>\n");
			printf("    getsize <filename>\n");
			printf("    copyin  <file name in host system> <miei02-filename>\n");
			printf("    copyout <miei02-filename> <file name in host system>\n");
			printf("	dump <number_of_block_with_text_contents>\n");
			printf("    help\n");
			printf("    quit\n");
			printf("    exit\n");
		} else if(!strcmp(cmd,"quit")) {
			break;
		} else if(!strcmp(cmd,"exit")) {
			break;
		} else if (!strcmp(cmd, "dump")){
			if(args==2) {
				int blNo = atoi(arg1);
				printf("Dumping disk block %d\n", blNo);
				char b[4096];
				disk_read( blNo, b);
				printf("------------------------------\n");
				printf("%s", b);
				printf("\n------------------------------\n");
			}
			else {
				printf("use: dump <block_number>\n");
			}
		} else {
			printf("unknown command: %s\n",cmd);
			printf("type 'help' for a list of commands.\n");
			result = 1;
		}
	}

	printf("closing emulated disk.\n");
	disk_close();

	return 0;
}

int do_copyin( char *filename, char *myfs_filename )
{
	FILE *file;
	int offset=0, result, actual;
	char buffer[16384];

	file = fopen(filename,"r");
	if(!file) {
		printf("couldn't open %s: %s\n",filename,strerror(errno));
		return 0;
	}

	while(1) {
		result = fread(buffer,1,sizeof(buffer),file);
		if(result<=0) break;
		if(result>0) {
			actual = fs_write(myfs_filename,buffer,result,offset);
			if(actual<0) {
				printf("ERROR: fs_write return invalid result %d\n",actual);
				break;
			}
			offset += actual;
			if(actual!=result) {
				printf("WARNING: fs_write only wrote %d bytes, not %d bytes\n",actual,result);
				break;
			}
		}
	}

	printf("%d bytes copied\n",offset);

	fclose(file);
	return 1;
}

int do_copyout( char *myfs_filename, char *filename )
{
	FILE *file;
	int offset=0, result;
	char buffer[16384];
    if(strcmp(filename,"/dev/stdout"))
		file = fopen(filename,"w");
	else
		file = stdout;
	if(!file) {
		printf("couldn't open %s: %s\n",filename,strerror(errno));
		return 0;
	}

	while(1) {
		result = fs_read(myfs_filename,buffer,sizeof(buffer),offset);
		if(result<=0) break;
		fwrite(buffer,1,result,file);
		offset += result;
	}

	printf("%d bytes copied\n",offset);

	if(file != stdout)
		fclose(file);
	return 1;
}

