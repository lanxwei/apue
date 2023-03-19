#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>

#define STR "I'm server"

int main(int argc,char **argv)
{
	int     rv=-1;
	int     socket_fd=-1;
	int     client_fd=-1;

	char    buf[1024];
	char    SOCK_PATH[108];

	socklen_t addrlen=sizeof(struct sockaddr_un);
	struct sockaddr_un serverAddr;
	struct sockaddr_un clientAddr;


	if(argc != 2)
	{
		printf("please input file name!\n");
		return -1;
	}

	snprintf(SOCK_PATH,strlen(argv[1])+1,"%s",argv[1]);

	socket_fd=socket(AF_UNIX,SOCK_STREAM,0);
	if(socket_fd < 0)
	{
		printf("socket failure:%s\n",strerror(errno));
		return -2;
	}

	memset(&serverAddr,0,sizeof(struct sockaddr_un));
	serverAddr.sun_family=AF_UNIX;
	serverAddr.sun_path[0]=0;
	snprintf(serverAddr.sun_path+1,strlen(SOCK_PATH)+1,"%s",SOCK_PATH);
	printf("sun_path:%s\n",serverAddr.sun_path+1);

	rv=bind(socket_fd,(struct sockaddr *)&serverAddr,addrlen);
	if(rv < 0)
	{
		printf("bind failure:%s\n",strerror(errno));
		return -3;
	}

	listen(socket_fd,13);

	client_fd=accept(socket_fd,(struct sockaddr *)&clientAddr,&addrlen);
	if(client_fd < 0)
	{
		printf("accept failure:%s\n",strerror(errno));
		return -4;
	}
	printf("accept client_fd[%d]\n",client_fd);
	
	while(1)
	{
		memset(buf,0,sizeof(buf));
		rv=read(client_fd,buf,sizeof(buf));
		if(rv <= 0)
		{
			printf("read failure:%s\n",strerror(errno));
			close(client_fd);
			break;
		}

		printf("read %d word:%s\n",rv,buf);
	}
	close(socket_fd);
	return 0;
}
