#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define  MSG_STR_C "child transferring information to parent"
#define  MSG_STR_P "parent transferring information to child"

int main(int argc,char **argv)
{
	pid_t   pid;
	int     fd[2];
	int     rv=0;
	char    buf[1024];

	memset(fd,0,sizeof(fd));
	memset(buf,0,sizeof(buf));
	pipe(fd);
	pid=fork();
	if(pid < 0)
	{
		printf("fork failure:%s\n",strerror(errno));
		return 0;
	}
	if(pid == 0)
	{
		sleep(2);
		printf("child start running\n");
		rv=write(fd[1],MSG_STR_C,strlen(MSG_STR_C));
		if(rv <= 0)
		{
			printf("child failure:%s\n",strerror(errno));
			goto cleanup;
		}
		sleep(1);
		rv=read(fd[0],buf,sizeof(buf));
		if(rv <= 0)
		{
			printf("child failure:%s\n",strerror(errno));
			goto cleanup;
		}
		printf("read %d char:%s\n",rv,buf);
	}
	if(pid > 0)
	{
		sleep(2);
		printf("parent start running\n");
		rv=read(fd[0],buf,sizeof(buf));
		if(rv <= 0)
		{
			printf("parent failure:%s\n",strerror(errno));
			goto cleanup;
		}
		printf("read %d char:%s\n",rv,buf);
		sleep(1);
		rv=write(fd[1],MSG_STR_P,strlen(MSG_STR_P));
		if(rv <= 0)
		{
			printf("parent failure:%s\n",strerror(errno));
			goto cleanup;
		}

	}

cleanup:
	close(fd[0]);
	close(fd[1]);
	return 0;
}
