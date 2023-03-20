#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include <netdb.h>
#include <signal.h>

#define Temp "temperature"


int  DS18b20(float *temp);
int  server_connect(char *server_ip,char *server_name,int port,int *connect_fd);
void print_usage(char *program);
void client_close(int signum);
void server_die(int signum);

int  g_sigstop=0;

int main(int argc,char **argv)
{
	int                 connect_fd=-1;
	int                 port;
	int                 rv=0;
	int                 rv_ds=0;

	float               temp=0;

	struct sockaddr_in  server_addr;
	char                *server_ip=NULL;
	char                *server_name=NULL;
	char                buf[1024];

	struct option       long_options[]={
		{"ipaddr",required_argument,NULL,'i' },
		{"server_name",required_argument,NULL,'n'},
		{"port",required_argument,NULL,'p' },
		{"help",no_argument,NULL,'h' },
		{0,0,0,0}
	};

	int                 ch;
	int                 idx;

	while((ch=getopt_long(argc,argv,"i:n:p:h",long_options,&idx)) != -1)
	{
		switch(ch)
		{
			case 'i':
				server_ip=optarg;
				break;
			case 'n':
				server_name=optarg;
				break;
			case 'p':
				port=atoi(optarg);
				break;
			case 'h':
				print_usage(argv[0]);
				break;
		}
	}
	if(NULL == argv[1] || NULL == argv[2])
	{
		printf("Missing required parameter!");
		return -1;
	}

	rv=server_connect(server_ip,server_name,port,&connect_fd);

	if(0 != rv)
	{
		printf("Connect server failure in:%d\n",rv);
	}
	while(!g_sigstop)
	{
		signal(SIGINT,client_close);
		rv_ds=DS18b20(&temp);
		snprintf(buf,sizeof(buf),"%f",temp);
		if(0 != rv_ds)
		{
			printf("DS18b20 get failure in the %d step\n",rv_ds);
			return -4;
		}

		if(write(connect_fd,buf,sizeof(buf))<0)
		{
			printf("Write failure:%s\n",strerror(errno));
			goto cleanup;
		}
		memset(buf,0,sizeof(buf));

		if((read(connect_fd,buf,sizeof(buf)))<0)
		{
			printf("Read failure:%s\n",strerror(errno));
			goto cleanup;
		}
		printf("Read from server:'%s'\n\n",buf);

		sleep(5);
	}
cleanup:
	signal(SIGPIPE,server_die);
	close(connect_fd);

	return 0;
}
void print_usage(char *program)
{
	printf("%s: usage \n",program);
	printf("-i(--ipaddr):server IP address.\n)");
	printf("-p(--port):server port.\n");
	printf("-h(--help):this is help information.\n");
	printf("-n(--server_name):server name");

}
int server_connect(char *server_ip,char *server_name,int port,int *connect_fd)
{
	int                 rc=0;
	int                 rv=0;
	int                 fd=-1;
	char                buf[128];
	socklen_t           addrlen=sizeof(struct sockaddr_in);
	struct sockaddr_in  server_addr;
	struct addrinfo     *result=NULL;
	struct addrinfo     hints={AI_CANONNAME,AF_INET,SOCK_STREAM,IPPROTO_TCP,0,NULL,NULL,NULL};

	memset(&server_addr,0,sizeof(server_addr));
	if(NULL != server_name)
	{
		rc=getaddrinfo(server_name,NULL,&hints,&result);

		if(0 == rc)
		{
			struct addrinfo     *res=result;

			while(res)
			{
				server_ip=inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr);
				printf("\"weike-iot.com\" IP:%s\n", server_ip);
				res=res->ai_next;
			}
			freeaddrinfo(result);
		}
	}

	if((*connect_fd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("socket failure:%s\n",strerror(errno));
		rv=-1;
	}
	printf("Create socket[%d] successfully\n\n",*connect_fd);

	server_addr.sin_family = AF_INET;
	printf("1\n");
	server_addr.sin_port = htons(port);
	printf("1\n");
	inet_aton(server_ip,&server_addr.sin_addr);

	printf("1\n");
	if(connect((int)*connect_fd,(struct sockaddr *)&server_addr,addrlen)<0)
	{
		printf("Connect failure:%s\n",strerror(errno));
		rv=-2;
	}
	return rv;

}
int DS18b20(float *temp)
{
	int      rv=0;
	FILE     *fp;
	char     buf[128];
	char     *ptr;

	memset(buf,0,sizeof(buf));
	snprintf(buf,sizeof(buf),"./DS18b20");
	if(NULL == (fp=popen(buf,"r")))
	{
		printf("popen() to excute command \"%s\" failure:%s\n",buf, strerror(errno));
		rv=-1;
		return rv;
	}
	while(fgets(buf,sizeof(buf),fp))
	{
		if(ptr=strstr(buf,Temp))
		{
			ptr=ptr+strlen(Temp)+1;
			break;
		}
	}

	*temp=atof(ptr);
	printf("DS18b20 temperature:%f\'C\n",*temp);
	return rv;
}

void client_close(int signum)
{
	printf("SIGINT signal detected\n");
	g_sigstop=1;
	exit(0);
}
void server_die(int signum)
{
	printf("SIGPIPE signal detected\n");
	g_sigstop=1;
	exit(0);
}
