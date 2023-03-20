#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>


#define PATH_NAME "fifo.c"
#define PROJ_ID   0x66

union semun
{ 
	int val; 
	struct semid_ds * buf; 
	unsigned short * array; 
	struct seminfo * __buf; 
};

int semaphore_init(void);
int semop_p(int semid);
int semop_v(int semid);
int semaphore_exit(int semid);

int main(int argc,char **argv)
{
	int    rv=-1;
	int    semid=-1;
	pid_t  pid=1;;

	semid=semaphore_init();
	if(semid < 0)
	{
		printf("semaphore failed in %d\n",semid);
		return -1;
	}
	pid=fork();

	if(pid < 0)
	{
		printf("fork failure:%s\n",strerror(errno));
		return -2;
	}
	if(pid == 0)
	{
		printf("red right on please wiat 3s\n");
		sleep(3);
		rv=semop_v(semid);
		if(rv < 0)
		{
			printf("faild in %d\n",rv);
			return -3;
		}
		printf("red right off\n");
		exit(0);

	}
	if(pid > 0)
	{
		printf("wait red right\n");
		rv=semop_p(semid);
		{
			printf("faild in %d\n",rv);
			return -3;
		}
		printf("green right on please go\n");
		semaphore_exit(semid);
		return 0;


	}
}

int semaphore_init(void)
{
	key_t  key=-1;
	int    semid=-1;
	union semun semun_x;
	key=ftok(PATH_NAME,PROJ_ID);
	if(key < 0)
	{
		printf("ftok failure:%s\n",strerror(errno));
		return -1;
	}
	printf("key:%d\n",key);
	semid=semget(key,1,IPC_CREAT | 0666);
	if(semid < 0)
	{
		printf("semget failure:%s\n",strerror(errno));
		return -2;
	}
	printf("semid:%d\n",semid);
	semun_x.val=0;
	if( semctl(semid,0,SETVAL,semun_x) == -1)
	{
		printf("semctl failure:%s\n",strerror(errno));
		return -3;
	}
	return semid;

}

int semop_p(int semid)
{
	struct sembuf sops;
	sops.sem_num=0;
	sops.sem_op=-1;
	sops.sem_flg=SEM_UNDO;
	if(semop(semid,&sops,1))
	{
		printf("semop failure:%s\n",strerror(errno));
		return -1;
	}
	return 0;

}
int semop_v(int semid)
{
	struct sembuf sops;
	sops.sem_num=0;
	sops.sem_op=1;
	sops.sem_flg=SEM_UNDO;
	if(semop(semid,&sops,1))
	{
		printf("semop failure:%s\n",strerror(errno));
		return -1;
	}
	return 0;

}

int semaphore_exit(int semid)
{
	union semun semun_y;

	if(semctl(semid,0,IPC_RMID,semun_y) < 0)
	{
		printf("semctl end failure:%s\n",strerror(errno));
		return -1;
	}
	return 0;
}







