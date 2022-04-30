#ifndef __VFS_H__
#define __VFS_H__

#include <dev/device.h>
#include <drivers/rtc.h>
#include <mm/vmm.h>
#include <stddef.h>
#include <stdint.h>
#include <vec.h>

#define F_DUPFD 1

#define R_OK 0x4
#define W_OK 0x2
#define X_OK 0x1
#define F_OK 0x0

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR 0x4
#define O_CREAT 0x200
#define O_DIRECTORY 0x200000
#define O_EXEC 0x400000
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)

#define S_IFMT 0170000
#define S_IFSOCK 0140000
#define S_IFLNK 0120000
#define S_IFREG 0100000
#define S_IFBLK 060000
#define S_IFDIR 040000
#define S_IFCHR 020000
#define S_IFIFO 010000

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)

#define S_IRGRP 040
#define S_IWGRP 020
#define S_IXGRP 010
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IROTH 04
#define S_IWOTH 02
#define S_IXOTH 01
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define S_ISCHR(n) ((n & S_IFMT) == S_IFCHR)
#define S_ISBLK(n) ((n & S_IFMT) == S_IFBLK)
#define S_ISDIR(n) ((n & S_IFMT) == S_IFDIR)
#define S_ISREG(n) ((n & S_IFMT) == S_IFREG)
#define ISDEV(file) (S_ISCHR((file->mode)) || S_ISBLK((file->mode)))
#define ISFIFO(file) ((file->mode & S_IFMT) == S_IFIFO)

struct pipe;

typedef signed long long int ssize_t;

typedef struct stat {
  uint64_t device_id;
  uint16_t file_serial;
  uint32_t file_mode;
  uint16_t hardlink_count;
  uint16_t user_id;
  uint16_t group_id;
  uint64_t device_type;
  size_t file_size;

  posix_time_t last_access_time;
  posix_time_t last_modification_time;
  posix_time_t last_status_change_time;
  posix_time_t creation_time;

  size_t block_size;
  size_t block_count;
} stat_t;

typedef struct dirent {
  char name[512];
  uint64_t file_serial;
} dirent_t;

// This has to have symlinks added but I'll do that a bit later

typedef struct fs_ops {
  char *fs_name;
  struct fs *(*mount)(device_t *dev);
  struct fs_file *(*open)(struct fs *fs, char *name);
  struct fs_file *(*mkdir)(struct fs *fs, char *path, int mode, int uid,
                           int gid);
  struct fs_file *(*create)(struct fs *fs, char *path, int mode, int uid,
                            int gid);
  struct fs_file *(*mkfifo)(struct fs *fs, char *path, int mode, int uid,
                            int gid);
  struct fs_file *(*mknod)(struct fs *fs, char *path, int mode, int uid,
                           int gid, device_t *dev);
} fs_ops_t;

typedef struct file_ops {
  ssize_t (*read)(struct fs_file *file, uint8_t *buf, size_t offset,
                  size_t count);
  ssize_t (*write)(struct fs_file *file, uint8_t *buf, size_t offset,
                   size_t count);
  int (*rmdir)(struct fs_file *file);
  int (*delete)(struct fs_file *file);
  int (*truncate)(struct fs_file *file, size_t size);
  int (*close)(struct fs_file *file);
  int (*readdir)(struct fs_file *file, dirent_t *dirent, size_t pos);
  int (*chmod)(struct fs_file *file, int mode);
  int (*chown)(struct fs_file *file, int uid, int gid);
  uint64_t (*ioctl)(struct fs_file *file, uint64_t cmd, void *arg);
  void *(*mmap)(struct fs_file *file, pagemap_t *pg, syscall_file_t *sfile,
                void *addr, size_t size, size_t offset, int prot, int flags);
} file_ops_t;

typedef struct fs {
  fs_ops_t *ops;
  device_t *dev;
  void *private_data;
} fs_t;

typedef struct fs_file {
  char *path;

  int uid;
  int gid;

  file_ops_t *file_ops;

  fs_t *fs;
  device_t *dev;
  struct pipe *pipe;

  posix_time_t last_access_time;
  posix_time_t last_modification_time;
  posix_time_t last_status_change_time;
  posix_time_t creation_time;

  int mode;
  size_t length;
  uint64_t inode;
  void *private_data;
  size_t offset;
} fs_file_t;

typedef struct vfs_mount_info {
  char *path;
  device_t *dev;
  fs_t *fs;
} vfs_mount_info_t;

void vfs_register_fs(fs_ops_t *ops);
int vfs_check_can_read(fs_file_t *file, int uid, int gid);
int vfs_check_can_write(fs_file_t *file, int uid, int gid);
int vfs_check_can_exec(fs_file_t *file, int uid, int gid);
uint64_t vfs_ioctl(fs_file_t *file, uint64_t cmd, void *arg);
void *vfs_mmap(fs_file_t *file, pagemap_t *pg, syscall_file_t *sfile,
               void *addr, size_t size, size_t offset, int prot, int flags);
int vfs_mount(char *path, device_t *dev, char *fs_name);
ssize_t vfs_read(fs_file_t *file, uint8_t *buf, size_t offset, size_t count);
ssize_t vfs_write(fs_file_t *file, uint8_t *buf, size_t offset, size_t count);
int vfs_rmdir(fs_file_t *file);
int vfs_delete(fs_file_t *file);
int vfs_truncate(fs_file_t *file, size_t size);
int vfs_fstat(fs_file_t *file, stat_t *stat);
int vfs_close(fs_file_t *file);
int vfs_readdir(fs_file_t *file, dirent_t *dirent, size_t pos);
int vfs_chmod(fs_file_t *file, int mode);
int vfs_chown(fs_file_t *file, int uid, int gid);
fs_file_t *vfs_mknod(char *path, int mode, int uid, int gid, device_t *dev);
fs_file_t *vfs_mkdir(char *path, int mode, int uid, int gid);
fs_file_t *vfs_create(char *path, int mode, int uid, int gid);
fs_file_t *vfs_mkfifo(char *name, int mode, int uid, int gid, int named);
fs_file_t *vfs_open(char *name);

int init_vfs();

#endif
