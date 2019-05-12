#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>

static const char *dirpath = "/home/zicoritonda/Music";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	(void) offset;
	(void) fi;

	//list(fpath);

	DIR *dp;
	struct dirent *de;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

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

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

static int check_if_mp3_file(const char *filename)//dapat di: https://sourceforge.net/p/mpg123/mailman/message/34642389/
{
        FILE *f=NULL;
        f=fopen(filename, "rb");
        if(f==NULL)
        {
                return 0;
        }

        //according to https://en.wikipedia.org/wiki/MP3#File_structure
        unsigned char bytes_pattern[2]={0xff,0xfb};
        unsigned char bytes_pattern_with_id3[3]={0x49,0x44,0x33};

        unsigned char bytes_header[3];

        size_t size=fread(bytes_header, 1, 3, f);

        if(size<3)
        {
                fclose(f);
                return 0;
        }

        if(bytes_header[0] == bytes_pattern[0] && bytes_header[1] == bytes_pattern[1])
        {
                fclose(f);
                return 1;
        }
        else if(bytes_header[0] == bytes_pattern_with_id3[0]
                && bytes_header[1] == bytes_pattern_with_id3[1]
                && bytes_header[2] == bytes_pattern_with_id3[2])
        {
                fclose(f);
                return 1;
        }
        fclose(f);
        return 0;
}	

void list(const char *path){
	DIR *dp;
	struct dirent *de;
	
	if (!(dp = opendir(path)))
        return;

	while ((de = readdir(dp)) != NULL) {
		if(de->d_type == DT_DIR){
			printf("Folder: %s\n",de->d_name);
			char newpath[1024];
			if(strchr(de->d_name,'.') != NULL || strcmp(de->d_name, "Music") == 0) continue;
			sprintf(newpath,"%s/%s",path,de->d_name);
			list(newpath);
		}
		else {
			printf("File: %s\n",de->d_name);
			char *point;
			char newname[500];
			sprintf(newname,"%s/%s",path,de->d_name);
			if((point = strstr(de->d_name,".mp3")) != NULL){
				if(check_if_mp3_file(newname)==1){
					char buf[1024];
					sprintf(buf,"cp '%s' '%s/%s'",newname,dirpath,de->d_name);
					system(buf);
					printf("\n Berhasil : %s\n",de->d_name);
				}
			}
		}
	}

	closedir(dp);
}

int main(int argc, char *argv[])
{
	list("/home/zicoritonda");
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
