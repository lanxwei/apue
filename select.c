#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/select.h>

#define MSG_STR       "Temperature received"
#define server_port   7777
#define ARRAY_SIZE(x) ((sizeof(x))/sizeof(x[0]))   

int Listen(int *listen_fd);
void signal_received(int signum);

int main(int argc,char **argv)
{
	int                fd=-1;
	int                listen_fd=-1;
	int                client_fd=-1;
	int                rv=0;
	pid_t              pid=0;
	char               buf[1024];
	socklen_t          cliaddr_len=sizeof(struct sockaddr);

	struct sockaddr_in cilent_addr;
	struct timeval     timeout;

	signal(SIGCHLD,signal_received);
	signal(SIGPIPE,signal_received);
	memset(&cilent_addr,0,sizeof(cilent_addr));

	Listen(&listen_fd);
	while(1)
	{
		client_fd=accept(listen_fd,(struct sockaddr*)&cilent_addr,&cliaddr_len);
		if(client_fd <0 && client_fd == 0)
		{
			printf("accept failure:%s\n",strerror(errno));
			continue;
		}
		printf("connect successfuly!\n");

		pid=fork();

		if(pid < 0)
		{
			printf("fork failed,the failure:%s\n",strerror(errno));
			exit(0);
		}

		if(pid == 0)
		{
			sleep(2);
			printf("[%d] is work\n",getpid());
			while(1)
			{
				rv=read(client_fd,buf,sizeof(buf));
				if(rv < 0)
				{
					printf("read filure:%s\n",strerror(errno));
					close(client_fd);
					return 0;
				}
				if(rv == 0 )
				{
					printf("connect with client error!\n");
					close(client_fd);
					return 0;
				}
				printf("\nget Temperature is :%s\'C\n",buf);
				memset(buf,0,sizeof(buf));
				rv=write(client_fd,MSG_STR,strlen(MSG_STR));
				if(rv < 0)
				{
					printf("write failure:%s\n",strerror(errno));
					close(client_fd);
					return 0;
				}
				if(rv == 0 )
				{
					printf("connect with client error!\n");
					close(client_fd);
					return 0;
				}
			}
		}
		if(pid > 0)
		{
			printf("child process[%d] start\n",pid);
		}
		close(listen_fd);
		return 0;
	}
}


int Listen(int *listen_fd)
{
	int           rv=0;
	int           on=1;

	socklen_t     cliaddr_len=sizeof(struct sockaddr); 

	struct        sockaddr_in server_addr;

	if((*listen_fd=socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("socket failure:%s\n",strerror(errno));
		rv=-1;
	}

	if(setsockopt(*listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0)
	{
		printf("setsockopt failure:%s\n",strerror(errno));
	}

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(server_port);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if((bind((int)*listen_fd,
					(struct sockaddr*)&server_addr,sizeof(server_addr)))<0)
	{
		printf("bind failure:%s\n",strerror(errno));
		rv=-2;
	}

	if(listen((int)*listen_fd,100)<0)
	{
		printf("listen failure:%s\n",strerror(errno));
		rv=-3;
	}

	if(0 == rv)
	{
		printf("listen successfuly\n");
	}
	else
	{
		printf("listen failure\n");
	}
	return rv;

}


void signal_received(int signum)
{
	if(signum == SIGPIPE)
	{
		printf("Client Force Discnnect!\n");
	}
	if(signum == SIGCHLD)
	{
		while(waitpid(-1,NULL,WNOHANG)>0)
		{
			printf("collect corpses for child process\n");
			printf("wait new connect:\n");
		}
	}
}

