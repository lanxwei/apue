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
#include <sys/epoll.h>

#define MSG_STR       "Temperature received"
#define MAX_SIZE      512
#define server_port   7777
#define ARRAY_SIZE(x) ((sizeof(x))/sizeof(x[0]))   

int Listen(int *listen_fd);
void signal_received(int signum);

int main(int argc,char **argv)
{
	int                fd=-1;
	int                listen_fd=-1;
	int                client_fd=-1;
	int                i=0;
	int                rv=0;
	char               buf[1024];

	socklen_t          cliaddr_len=sizeof(struct sockaddr);
	struct sockaddr_in client_addr;

	int                epfd=-1;
	int                events=-1;
	struct epoll_event        event;
	struct epoll_event        epoll_events[MAX_SIZE];

	signal(SIGCHLD,signal_received);
	signal(SIGPIPE,signal_received);
	memset(&client_addr,0,sizeof(client_addr));

	Listen(&listen_fd);

	epfd=epoll_create(MAX_SIZE);
	if(epfd < 0)
	{
		printf("epoll_create failure:%s\n",strerror(errno));
		exit(0);
	}

	event.events = EPOLLIN;
	event.data.fd = listen_fd;

	if(epoll_ctl(epfd,EPOLL_CTL_ADD,listen_fd,&event) < 0)
	{
		printf("epoll_ctl listen_fd add failure:%s\n",strerror(errno));
		exit(0);
	}

	while(1)
	{
		events=epoll_wait(epfd,epoll_events,MAX_SIZE,-1);
		if(events < 0)
		{
			printf("epoll_wait is filed,failure:%s\n",strerror(errno));
			break;
		}
		if(events == 0)
		{
			printf("epoll_wait connect timeout");
			continue;
		}
		for(i=0;i<events;i++)
		{
			if((epoll_events[i].events & EPOLLERR) || epoll_events[i].events & EPOLLHUP)
			{
				printf("epoll_wait is faild,the fd[%d]\n",epoll_events[i].data.fd);
				close(epoll_events[i].data.fd);
				continue;
			}
			if(epoll_events[i].data.fd == listen_fd)
			{
				client_fd = accept(listen_fd,(struct sockaddr *)&client_addr,&cliaddr_len);
				if(client_fd <= 0 )
				{
					printf("accept failure:%s/n",strerror(errno));
					continue;
				}
				printf("connect successfully!\n");

				event.events = EPOLLIN;
				event.data.fd = client_fd;
				epoll_ctl(epfd,EPOLL_CTL_ADD,client_fd,&event);
			}
			else
			{
				rv=read(epoll_events[i].data.fd,buf,sizeof(buf));
				if(rv < 0)
				{
					printf("read filure:%s\n",strerror(errno));
					epoll_ctl(epfd,EPOLL_CTL_DEL,epoll_events[i].data.fd,NULL);
					close(epoll_events[i].data.fd);
					break;
				}
				if(rv == 0)
				{
					printf("connect with client error!\n");
					epoll_ctl(epfd,EPOLL_CTL_DEL,epoll_events[i].data.fd,NULL);
					close(epoll_events[i].data.fd);
					break;
				}
				printf("\nget Temperature is :%s\'C\n",buf);
				memset(buf,0,sizeof(buf));
				rv=write(epoll_events[i].data.fd,MSG_STR,strlen(MSG_STR));
				if(rv < 0)
				{
					printf("write failure:%s\n",strerror(errno));
					epoll_ctl(epfd,EPOLL_CTL_DEL,epoll_events[i].data.fd,NULL);
					close(epoll_events[i].data.fd);
					break;
				}
				if(rv == 0)
				{
					printf("connect with client error!\n");
					epoll_ctl(epfd,EPOLL_CTL_DEL,epoll_events[i].data.fd,NULL);
					close(epoll_events[i].data.fd);
					break;
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

