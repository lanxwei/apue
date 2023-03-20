#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<string.h>
#include<time.h>
#include<errno.h>

#define w1_path "/sys/bus/w1/devices/"

int Temperature(float *temp)
{
	char buf[100];
	DIR  *dirp;
	struct dirent *direntp;
	char chip[64];
	char ds_path[128];
	int fd = -1;
	int rv=0;
	int found=0;
	char *ptr;

	if(!temp)
	{
		return -1;
	}


	if(( dirp = opendir(w1_path)) == NULL )
	{
		printf("Open failue:%s\n",strerror(errno));
		return -2;
	}

	while( (direntp = readdir( dirp )) != NULL )
	{
		if( strstr(direntp->d_name,"28-"))
		{
			strcpy(chip,direntp->d_name);
			found=1;
			break;
		}
	}

	closedir(dirp);

	if(found != 1)
	{
		printf("Can not find ds18b20 in %s\n",w1_path);
		return -3;

	}

	snprintf(ds_path,sizeof(ds_path),"%s/%s/w1_slave",w1_path,chip);


	if( (fd = open(ds_path,O_RDONLY)) < 0 )
	{
		printf("the open failue:%s\n",strerror(errno));
		return -4;
	}

	if( read(fd,buf,sizeof(buf)) < 0 )
	{
		printf("the read failue:%s\n",strerror(errno));
		rv = -5;
		goto cleanup;
	}

	ptr = strstr(buf,"t=");
	if(!ptr)
	{
		printf("Can not get Temperature\n");
		rv=-6;
		goto cleanup;
	}
	ptr+=2;
	*temp = atof(ptr)/1000;
cleanup:
	close(fd);
	return rv;
}
int Temperature(float *temp);

int main()
{
	float temp;
	Temperature(&temp);
	printf("the Temperature is %f 'C\n",temp);
	return 0;
}
