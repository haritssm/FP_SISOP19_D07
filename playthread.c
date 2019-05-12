#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<dirent.h>
#include<ao/ao.h>
#include<mpg123.h>

#define BITS 8

int x;
char now[500];
char before[500];
static const char *dirpath = "/home/zicoritonda/MP3";

mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

pthread_t tid[2];

void list()
{
	DIR *dp;
	struct dirent *de;
	
	if (!(dp = opendir(dirpath)))
        return;
    int i=1;
	while ((de = readdir(dp)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		printf("%d. %s\n",i,de->d_name);
		i++;
	}
	closedir(dp);
}

void next(const char *file){
	DIR *dp;
	struct dirent *de;
	
	if (!(dp = opendir(dirpath)))
        return;

    int flag=0;
	while ((de = readdir(dp)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		
		//next
		if(flag==1){
			strcpy(now,de->d_name);
			break;
		}
		if(strcmp(file,de->d_name)==0){
			flag=1;
			strcpy(before,de->d_name);
			continue;
		}
	}
	flag=0;
	closedir(dp);
}

void prev(const char *file){
	DIR *dp;
	struct dirent *de;
	
	if (!(dp = opendir(dirpath)))
        return;

	while ((de = readdir(dp)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

		//prev
		if(strcmp(file,de->d_name)==0){
			break;
		}
		else strcpy(before,de->d_name);
	}
	strcpy(now,before);
	closedir(dp);
}

void preplay(){
    char filename[500];
    sprintf(filename,"%s/%s",dirpath,now);

    /* open the file and get the decoding format */
    mpg123_open(mh, filename);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
}

void* play(void *args){
	while(x!=1){
			if(mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) ao_play(dev, buffer, done);
	}
}

void* in(void *args){
	char str[100];
	while(1){
		if(scanf("%s",str)==1){
			if(strcmp(str,"exit")==0){
				x=1;
				break;
				//exit(0);
			}
			//if(strcmp(str,"play")==0){
			//	if(system("clear")!=-1)
			//	printf("\nSedang dimainkan: %s\n",now);
			//}
			if(strcmp(str,"next")==0){
				//free(buffer);
				if(system("clear")!=-1)
				//system("clear");
				next(now);
				list();
				printf("\nSedang dimainkan: %s\n",now);
				preplay();
			}
			if(strcmp(str,"prev")==0){
				//free(buffer);
				//system("clear");
				if(system("clear")!=-1)
				prev(now);
				list();
				printf("\nSedang dimainkan: %s\n",now);
				preplay();
			}
			//if(strcmp(str,"list")==0){
			//	if(system("clear")!=-1)
			//	list();
			//	printf("\nSedang dimainkan: %s\n",now);
			//}
			sleep(1);
		}
	}
}

int main()
{
	x=0;

	/* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

	//cari lagu pertama
	DIR *dp;
	struct dirent *de;
	if (!(dp = opendir(dirpath)))
        return 0;
    int i=0;
	while ((de = readdir(dp)) != NULL) {
		if(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
		strcpy(now,de->d_name);
		break;
	}
	closedir(dp);

	if(system("clear")!=-1)
	list();
	printf("\nSedang dimainkan: %s\n",now);
	preplay();

	int err1,err2;

	err1 = pthread_create(&(tid[0]),NULL,&in,NULL);
	if(err1) exit(EXIT_FAILURE);
	err2 = pthread_create(&(tid[1]),NULL,&play,NULL);
	if(err2) exit(EXIT_FAILURE);

	pthread_join(tid[0],NULL);
	pthread_join(tid[1],NULL);

	/* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

	return 0;
}