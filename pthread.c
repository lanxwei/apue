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
#include <pthread.h>


#define MSG_STR       "Temperature received"
#define server_port   7778
#define ARRAY_SIZE(x) ((sizeof(x))/sizeof(x[0]))   

typedef struct pthread_data
{
	int   fd;
}pthread_data_s;

int  thread_start(int client_fd);
void *pthread_work(void *arg);
int  Listen(int *listen_fd);
void signal_received(int signum);

int main(int argc,char **argv)
{
	int                listen_fd = -1;
	int                client_fd = -1;
	int                rv = 0;
	int                fd_arry[1024];
	socklen_t          cliaddr_len=sizeof(struct sockaddr);

	struct sockaddr_in cilent_addr;

	signal(SIGINT,signal_received);
	signal(SIGPIPE,signal_received);
	memset(&cilent_addr,0,sizeof(cilent_addr));

	Listen(&listen_fd);
	while(listen_fd)
	{
		client_fd=accept(listen_fd,(struct sockaddr*)&cilent_addr,&cliaddr_len);
		if(client_fd < 0 && client_fd == 0)
		{
			printf("accept failure:%s\n",strerror(errno));
			continue;
		}
		printf("connect successfuly!\n");
		thread_start(client_fd);

	}

	close(listen_fd);
	return 0;
}

int thread_start(int client_fd)
{
	int            rv=-1;
	pthread_data_s pthread_data;
	pthread_t      thread=0;
	pthread_attr_t *attr;

	pthread_data.fd=client_fd;
	pthread_attr_init(attr);
	if(pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED) != 0)
	{
		printf("pthread init failure:%s\n",strerror(errno));
		rv = 0;
	}
	if(pthread_create(&thread,attr,pthread_work,&pthread_data) != 0)
	{
		printf("pthread_create failure:%s\n",strerror(errno));
		rv = 1;
	}

	return rv;
}

void *pthread_work(void *arg)
{
	int               rv = -1;
	int               fd = -1;
	char              buf[1024];	
	pthread_data_s    *pthread_data=(pthread_data_s *)arg;

	fd = pthread_data->fd;
	memset(buf,0,sizeof(buf));
	while(1)
	{
		rv=read(fd,buf,sizeof(buf));
		if(rv < 0 ) 
		{
			printf("read filure:%s\n",strerror(errno));
			close(fd);
			break;
		} 
		if(rv == 0 ) 
		{
			printf("connect with client error!\n");	
			close(fd);
			break;
		}
		printf("\nget Temperature from [%ld] is :%s\'C\n",pthread_self(),buf); 
		memset(buf,0,sizeof(buf));
		rv=write(fd,MSG_STR,strlen(MSG_STR));
		if(rv < 0 ) 
		{
			printf("write failure:%s\n",strerror(errno));
			close(fd);
			break;
		}
		if(rv == 0 )
		{
			printf("connect with client error!\n");
			close(fd);
			break;
		}
	}
	pthread_exit(0);
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

	if((bind((int)*listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)))<0)
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
		printf("Listen successfuly \nWait the client connect \n");
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
		printf("Connection interrupted!\n");
	}
	else if(signum == SIGINT)
	{
		printf("break link!\n");
		exit(0);
	}
}

