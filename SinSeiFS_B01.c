#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static  const  char *dir_path = "/home/[user]/Downloads";
char *log_path = "/home/[user]/SinSeiFS.log";

static  int  xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];

    sprintf(fpath,"%s%s",dir_path,path);

    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}



static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];

    if(strcmp(path,"/") == 0)
    {
        path=dir_path;
        sprintf(fpath,"%s",path);
    } else sprintf(fpath, "%s%s",dir_path,path);

    int res = 0;

    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    dp = opendir(fpath);

    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = (filler(buf, de->d_name, &st, 0));

        if(res!=0) break;
    }

    closedir(dp);

    return 0;
}



static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0)
    {
        path=dir_path;

        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath, "%s%s",dir_path,path);

    int res = 0;
    int fd = 0 ;

    (void) fi;

    fd = open(fpath, O_RDONLY);

    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);

    if (res == -1) res = -errno;

    close(fd);

    return res;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dir_path;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fpath, "%s%s", dir_path, temp);
  }

  res = mkdir(fpath, mode);
  if (res == -1)
    return -errno;

  char desc[100];
  sprintf(desc, "MKDIR::%s", fpath);
  writeLog("INFO", desc);

  return 0;
}

static int xmp_unlink(const char *path)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dir_path;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fpath, "%s%s", dir_path, temp);
  }

  res = unlink(fpath);
  if (res == -1)
    return -errno;

  char desc[100];
  sprintf(desc, "REMOVE::%s", fpath);
  writeLog("WARNING", desc);
  
  return 0;
}

static int xmp_rmdir(const char *path)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dir_path;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fpath, "%s%s", dir_path, temp);
  }
  res = rmdir(fpath);
  if (res == -1)
    return -errno;

  char desc[100];
  sprintf(desc, "RMDIR::%s", fpath);
  writeLog("WARNING", desc);

  return 0;
}

static int xmp_rename(const char *from, const char *to)
{
  int res;

  char ffrom[1000];
  if (strcmp(from, "/") == 0)
  {
    from = dir_path;
    sprintf(ffrom, "%s", from);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, from);

    if(strncmp(from, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(ffrom, "%s%s", dir_path, temp);
  }

  char fto[1000];
  if (strcmp(to, "/") == 0)
  {
    to = dir_path;
    sprintf(fto, "%s", to);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, to);

    if(strncmp(to, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fto, "%s%s", dir_path, temp);
  }

  res = rename(ffrom, fto);
  if (res == -1)
    return -errno;

  char desc[100];
  sprintf(desc, "RENAME::%s::%s", ffrom, fto);
  writeLog("INFO", desc);

  return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dir_path;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fpath, "%s%s", dir_path, temp);
  }

  res = truncate(fpath, size);
  if (res == -1)
    return -errno;

  return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
  int res;

  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dir_path;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fpath, "%s%s", dir_path, temp);
  }

  res = open(fpath, fi->flags);
  if (res == -1)
    return -errno;

  close(res);
  return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];

  if (strcmp(path, "/") == 0)
  {
    path = dir_path;
    sprintf(fpath, "%s", path);
  }
  else 
  {
    char temp[1000];
    strcpy(temp, path);

    if(strncmp(path, "/encv1_", 6) == 0)
      decryptV1(temp);

    sprintf(fpath, "%s%s", dir_path, temp);
  }

  int fd;
  int res;

  (void) fi;
  fd = open(fpath, O_WRONLY);
  if (fd == -1)
    return -errno;

  res = pwrite(fd, buf, size, offset);
  if (res == -1)
    res = -errno;

  close(fd);

  char desc[100];
  sprintf(desc, "WRITE::%s", fpath);
  writeLog("INFO", desc);

  return res;
}


static struct fuse_operations xmp_oper = {
    .getattr  = xmp_getattr,
    .readdir  = xmp_readdir,
    .read     = xmp_read,
    .mkdir    = xmp_mkdir,
    .unlink   = xmp_unlink,
    .rmdir    = xmp_rmdir,
    .rename   = xmp_rename,
    .truncate = xmp_truncate,
    .open     = xmp_open,
    .write    = xmp_write,
};



int  main(int  argc, char *argv[])
{
    umask(0);

    return fuse_main(argc, argv, &xmp_oper, NULL);
}
