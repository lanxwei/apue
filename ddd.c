#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

#define FTOK_PATH   "/dev/zero"
#define FTOK_PROJID  0x22
#define LEN  1024
typedef struct shared_use_st
{
	int written;
	char  T[LEN];
}shared_use;
int main(int argc,char **argv)
{
	float *temp;
	char    *w1_path = "/sys/bus/w1/devices";
	char    ds_path[50];
	char    chip[20];
	char    buf[1024];
	DIR     *dirp = 0;
	struct  dirent  *direntp;
	int     ds18b20_fd = -1; 
	char    *ptr = 0;
	int     found = 0;
	int     ds18b20_rv = 0;
	float   t = 0;

	key_t  key;
	int  shmid;
	shared_use  *shared ;

	if((key = ftok(FTOK_PATH,FTOK_PROJID)) < 0)
	{
		printf("ftok() get key failure:%s\n",strerror(errno));
		return -1;
	}
	shmid = shmget(key,sizeof(shared_use),IPC_CREAT | 0666);
	if(shmid < 0)
	{
		printf("shmget() create shared memory failure:%s\n",strerror(errno));
		return -1;
	}
	printf("get key_t[0x%x]  and shmid [%d]\n",key,shmid);

	shared = shmat(shmid,NULL,0);
	if((void *) -1 == shared)
	{
		printf("shmat() alloc shared memroy failure:%s\n",strerror(errno));
		return -1;
	}

	if((dirp = opendir(w1_path)) == NULL )
	{
		printf("opendir error: %s\n",strerror(errno));
		return -2;
	}

	while((direntp = readdir(dirp)) != NULL)
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strcpy(chip,direntp->d_name);
			found = 1;
			break;
		}
	}
	closedir(dirp);
	if(!found)
	{
		printf("can not find ds18b20 in %s\n",w1_path);
		return -3;
	}

	snprintf(ds_path,sizeof(ds_path),"%s/%s/w1_slave",w1_path,chip);
	if((ds18b20_fd = open(ds_path,O_RDONLY)) < 0 )
	{
		printf("open %s error : %s\n",ds_path,strerror(errno));
		return -4;
	}
	if(read(ds18b20_fd,buf,sizeof(buf)) < 0)
	{
		printf("read %s error:%s\n",ds_path,strerror(errno));
		ds18b20_rv = -5;
		goto cleanup;
	}

	ptr = strstr(buf,"t=");
	if(!ptr)
	{
		printf("error:can not get temperature\n");
		ds18b20_rv = -7;
		goto cleanup;
	}
	ptr += 2;
	*temp = atof(ptr)/1000;
	snprintf(shared->T,LEN,"%f",*temp);

	while(1)
	{
		while(shared->written == 1)
		{
			sleep(1);
			printf("waiting...\n");
		}
		printf("the temperature is:%s\n",shared->T);
		shared->written = 1;
	}
	shmdt(shared);
	shmctl(shmid,IPC_RMID,NULL);
	sleep(2);
	return 0;
cleanup:
	close(ds18b20_fd);
	return ds18b20_rv;
}
