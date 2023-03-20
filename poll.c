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
#define server_port   34444
#define ARRAY_SIZE(x) ((sizeof(x))/sizeof(x[0]))   

int Listen(int *listen_fd);
void signal_received(int signum);

int main(int argc,char **argv)
{
	int                fd=-1;
	int                listen_fd=-1;
	int                client_fd=-1;
	int                i=0;
	int                max_fd=0;
	int                rv=0;
	char               buf[1024];
	struct pollfd      fd_arry[1024];
	fd_set             readset;
	socklen_t          cliaddr_len=sizeof(struct sockaddr);

	struct sockaddr_in cilent_addr;
	struct timeval     timeout;

	signal(SIGCHLD,signal_received);
	signal(SIGPIPE,signal_received);
	memset(&cilent_addr,0,sizeof(cilent_addr));
	memset(fd_arry,-1,sizeof(fd_arry));
	timeout.tv_sec=30;
	timeout.tv_usec=0;

	Listen(&listen_fd);
	fd_arry[0].fd=listen_fd;
	fd_arry[0].events=POLLIN;
	for(;;)
	{

		for(i=0;i<ARRAY_SIZE(fd_arry);i++)
		{
			if(fd_arry[i].fd<0)
			{
				continue;
			}
			max_fd=fd_arry[i].fd>max_fd?fd_arry[i].fd:max_fd;
			fd_arry[i].events=POLLIN;

		}

		rv=poll(fd_arry,max_fd,&timeout);
		if(rv < 0)
		{
			printf("poll failure:%s\n",strerror(errno));
			break;
		}
		if(rv = 0)
		{
			printf("poll timed out");
			continue;
		}
		if(fd_arry[i]_revents & POLLIN)
		{
			client_fd=accept(listen_fd,(struct sockaddr*)&cilent_addr,&cliaddr_len);
			if(client_fd <0 && client_fd == 0)
			{
				printf("accept failure:%s\n",strerror(errno));
				continue;
			}
			printf("connect successfuly!\n");
			for(i=0;i<ARRAY_SIZE(fd_arry);i++)
			{
				if(fd_arry[i]<0)
				{
					fd_arry[i]=client_fd;
					break;
				}

			}
		}
		else
		{
			for(i=0;i<ARRAY_SIZE(fd_arry);i++)
			{
				if(fd_arry[i]>0  && FD_ISSET(fd_arry[i],&readset))
				{
					rv=read(fd_arry[i],buf,sizeof(buf));
					if(rv < 0)
					{
						printf("read filure:%s\n",strerror(errno));
						close(fd_arry[i]);
						fd_arry[i]=-1;
						break;
					}
					if(rv == 0 )
					{
						printf("connect with client error!\n");
						close(fd_arry[i]);
						fd_arry[i]=-1;
						break;
					}
					printf("\nget Temperature is :%s\'C\n",buf);
					memset(buf,0,sizeof(buf));
					rv=write(fd_arry[i],MSG_STR,strlen(MSG_STR));
					if(rv < 0)
					{
						printf("write failure:%s\n",strerror(errno));
						close(fd_arry[i]);
						fd_arry[i]=-1;
						break;
					}
					if(rv == 0 )
					{
						printf("connect with client error!\n");
						close(fd_arry[i]);
						fd_arry[i]=-1;
						break;
					}
				}
			}

		}

	}
	close(listen_fd);
	return 0;
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
		printf("Client Force Discnnect!");
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

