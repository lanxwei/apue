#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#define FIFO_FILE0 ".fifo_chat0"
#define FIFO_FILE1 ".fifo_chat1"

int main(int argc,char **argv)
{
	int rv=-1;
	int mode=-1;
	int read_fd=-1;
	int write_fd=-1;
	int ret=-1;
	fd_set rdset;
	char buf[1024];


	if(argc != 2)
	{
		printf("error! please choice one FIFO_FILE\n");
		return 0;
	}

	mode=atoi(argv[1]);
	printf("mode:%d\n",mode);

	if(access(FIFO_FILE0,F_OK) < 0) 
	{
		printf("file not exist");
		mkfifo(FIFO_FILE0,0666);
	}

	if(access(FIFO_FILE1,F_OK) < 0) 
	{
		printf("file not exist");
		mkfifo(FIFO_FILE1,0666);
	}

	if(mode == 0)
	{
		if((read_fd=open(FIFO_FILE0,O_RDONLY)) < 0)
		{
			printf("open FIFO_FILE0 failure:%s\n",strerror(errno));
			return 0;
		}
		if((write_fd=open(FIFO_FILE1,O_WRONLY)) < 0)
		{
			printf("open FIFO_FILE1 failure:%s\n",strerror(errno));
			return 0;
		}
	}

	if(mode == 1)
	{
		if((write_fd=open(FIFO_FILE0,O_WRONLY)) < 0)
		{
			printf("open FIFO_FILE0 failure:%s\n",strerror(errno));
			return 0;
		}
		if((read_fd=open(FIFO_FILE1,O_RDONLY)) < 0)
		{
			printf("open FIFO_FILE1 failure:%s\n",strerror(errno));
			return 0;
		}
	}

	while(1)
	{
		FD_ZERO(&rdset);
		FD_SET(STDIN_FILENO,&rdset);
		FD_SET(read_fd,&rdset);
		ret=select(read_fd+2,&rdset,NULL,NULL,NULL);
		if(ret <= 0)
		{
			printf("select failure:%s\n",strerror(errno));
			goto cleanup;
		}

		if(FD_ISSET(read_fd,&rdset))
		{
			memset(buf,0,sizeof(buf));
			rv=read(read_fd,buf,sizeof(buf));
			if(rv <= 0)
			{
				printf("read failure:%s\n",strerror(errno));
				goto cleanup;
			}

			printf("read %d word:%s\n",rv,buf);
		}
		if(FD_ISSET(STDIN_FILENO,&rdset))
		{
			memset(buf,0,sizeof(buf));
			rv=read(STDIN_FILENO,buf,sizeof(buf));
			if(rv <= 0)
			{
				printf("read failure:%s\n",strerror(errno));
				goto cleanup;
			}
			rv=write(write_fd,buf,strlen(buf));
			if(rv <= 0)
			{
				printf("write failure:%s\n",strerror(errno));
				goto cleanup;
			}

		}

	}
cleanup:
	close(read_fd);
	close(write_fd);
	return 0;
}
