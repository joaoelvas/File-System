// This is an Academic Project, and was published after finishing the lecture.
// @author Joao Elvas @ FCT/UNL

#ifndef FS_H
#define FS_H

void fs_debug();
int  fs_format();
int  fs_mount();

int  fs_create( char *name);
int  fs_delete( char *name );
int  fs_getsize( char *name);

int  fs_read( char *name, char *data, int length, int offset );
int  fs_write( char *name, const char *data, int length, int offset );

#endif
