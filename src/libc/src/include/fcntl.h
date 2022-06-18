#ifndef __FCNTL_H__
#define __FCNTL_H__

#define F_DUPFD 1

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR 0x4
#define O_CREAT 0x200
#define O_DIRECTORY 0x200000
#define O_EXEC 0x400000
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)

int open(char *pathname, int flags, ...);

#endif
