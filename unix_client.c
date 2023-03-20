#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>

#define STDIN 0

int main(int argc,char **argv)
{
	int    rv = -1;
	int    client_fd = -1;
	char   SOCK_PATH[108];
	char   buf[1024];

	struct sockaddr_un clientAddr;

	if(argc != 2)
	{
		printf("please input pathname!\n");
		return -1;
	}

	snprintf(SOCK_PATH,strlen(argv[1])+1,"%s",argv[1]);

	client_fd=socket(AF_UNIX,SOCK_STREAM,0);
	if(client_fd < 0)
	{
		printf("socket failure:%s\n",strerror(errno));
	}

	memset(&clientAddr,0,sizeof(struct sockaddr_un));
	clientAddr.sun_family = AF_UNIX;
	clientAddr.sun_path[0]=0;
	snprintf(clientAddr.sun_path+1,strlen(SOCK_PATH)+1,"%s",SOCK_PATH);

	if(connect(client_fd,(struct sockaddr *)&clientAddr,sizeof(struct sockaddr_un)) < 0)
	{
		printf("connect failure:%s\n",strerror(errno));
		return -2;
	}

	while(1)
	{
		memset(buf,0,sizeof(buf));
		rv=read(STDIN,buf,sizeof(buf));
		if(rv <= 0)
		{
			printf("STDIN read failure:%s\n",strerror(errno));
			close(client_fd);
			return -3;
		}

		rv=write(client_fd,buf,strlen(buf));
		if(rv <= 0)
		{
			printf("write failure:%s\n",strerror(errno));
			close(client_fd);
			return -3;
		}

	}
	return 0;




}
