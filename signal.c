#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int parent_num=0;
int child_num=0;

void signal_step1(int signum);
void signal_step2(int signum);

void signal_step1(int signum)
{
	if(signum == SIGUSR1)
	{
		parent_num=1;
	}
}

void signal_step2(int signum)
{
	if(signum == SIGUSR2)
	{
		child_num=1;
	}
}

int main(int argc,char **argv)
{

	pid_t    pid;

	signal(SIGUSR1,signal_step1);
	signal(SIGUSR2,signal_step2);

	pid=fork();

	if(pid < 0)
	{
		printf("fork failure:%s\n",strerror(errno));
		return 0;
	}

	while(1)
	{}
	if(pid == 0)
	{
		printf("child process strat\n");
		kill(getppid(),SIGUSR1);
		while(!child_num)
		{
			sleep(1);
		}
		printf("child process successfully\n");

		exit(0);

	}

	if(pid > 0)
	{
		printf("parent process start\n");
		kill(pid,SIGUSR2);
		while(!parent_num)
		{
			sleep(1);
		}
		printf("parent process successfully\n");
	}
	printf("task complete process quit\n");
}













