#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define FTOK_PATH "ksad"
#define PROJ_ID   0x22
#define size_buf  1024

int mq_init(void);
int mq_msgsnd(int msqid,char *ptr,size_t len_buf);
int mq_msgrcv(int msqid,char *ptr);

int stdin_read(char *ptr,size_t len_buf);

int main(int argc,char **argv)
{
	int    rv;
	long   rc;
	int    msqid;
	char   buf[1024];
	size_t len_buf;
	struct msqid_ds *msqid_buf;

	if((msqid=mq_init()) <0)
	{
		printf("mq_init error\n");
		return -1;
	}
	len_buf=sizeof(buf);
	while(1)
	{
		memset(buf,0,sizeof(buf));
/*		if((len_buf=stdin_read(buf,len_buf)) < 0)
		{
			printf("stdin_read error\n");
			goto cleanup;
		}
		printf("len_buf:%ld\n",len_buf);
*/
		if( (rv=mq_msgrcv(msqid,buf)) < 0)
		{
			printf("mq_msgrcv error\n");
			goto cleanup;
		}
		printf("len_buf:%ld\n",len_buf);

/*
		if((mq_msgsnd(msqid,buf,len_buf)) < 0)
		{
			printf("mq_msgsnd error\n");
			goto cleanup;

		}
*/
	}
cleanup:
	msgctl(msqid,IPC_RMID,msqid_buf);
	return 0;

}

int mq_init(void)
{
	int             msqid;
	key_t           key;
	struct msqid_ds *msqid_buf;

	if((key=ftok(FTOK_PATH,PROJ_ID)) < 0)
	{
		printf("fotk failure:%s\n",strerror(errno));
		return -1;
	}
	if((msqid=msgget(key,IPC_CREAT|0666)) == -1)
	{
		printf("msgget:%s\n",strerror(errno));
		return -2;
	}
	return msqid;
}

int mq_msgsnd(int msqid,char *ptr,size_t len_buf)
{
	if(msgsnd(msqid,ptr,len_buf,IPC_NOWAIT) == -1)
	{
		printf("msgsnd failure:%s\n",strerror(errno));
		return -1;
	}
	return 0;
}
int mq_msgrcv(int msqid,char *ptr)
{
	int  len_buf=-1;
	if((len_buf=msgrcv(msqid,ptr,size_buf,0,IPC_NOWAIT)) < 0)
	{
		printf("msgrcv failure:%s\n",strerror(errno));
		return -1;
	}
	printf("read %d char:%s\n",len_buf,ptr);
	return len_buf;

}

int stdin_read(char *ptr,size_t len_buf)
{
	int   rv=-1;
	printf("please input:\n");
	if((rv=read(0,ptr,len_buf)) < 0)
	{
		printf("read fialure:%s\n",strerror(errno));
		return -1;
	}
	return rv;
}
