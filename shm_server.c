#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>

#define PATH_NAME "ksad"
#define PROJ_ID   0x33

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
void semaphore_exit(int semid);

int shmaphore_init(char **shmptr);
void shmaphore_use(char *shmptr);
void shmaphore_exit(int shmid,char *shmptr); 

int main(int argc,char **argv)
{
	char   *ptr=NULL;
	int    rv=-1;
	int    shmid=-1;
	int    semid=-1;

	if((shmid=shmaphore_init(&ptr)) < 0)
	{
		printf("shmaphore_init error\n");
		goto cleanup;
	}
	if((semid=semaphore_init()) < 0) 
	{
		printf("semaphor_init error\n");
		goto cleanup;
	}

	semop_p(semid);
	shmaphore_use(ptr);
	semop_v(semid);

cleanup:
	shmaphore_exit(shmid,ptr);
	semaphore_exit(semid);

	return 0;
}

int shmaphore_init(char **shmptr)
{

	int    shmid;
	key_t  key;

	key=ftok(PATH_NAME,PROJ_ID);
	if(key < 0)
	{       
		printf("ftok failure:%s\n",strerror(errno));
		return -1;
	}
	printf("key:%d\n",key);
	shmid=shmget(key,4096,IPC_CREAT|0660);
	if(shmid < 0)
	{
		printf("shmget failure:%s\n",strerror(errno));
		return -2;
	}
	printf("shmid:%d\n",shmid);

	*shmptr=(char *)shmat(shmid,NULL,0);
	memset(*shmptr,0,4096);
	if(*shmptr == (char *)-1)
	{
		printf("shmat failure:%s\n",strerror(errno));
		return -3;
	}
	return shmid;
}

void shmaphore_use(char *shmptr)
{
	char    record=0;
	int     i=0;
	while(1)
	{
		printf("message from sharememory start\n");
		for(i=0;;i++)
		{
			if(*shmptr == record)
			{
				continue;
			}
			if(*shmptr == 0)
			{
				printf("over!\n");
				return;
			}
			printf("shmptr:%s\n",shmptr);
			record=*shmptr;
		}
	}
}

void shmaphore_exit(int shmid,char *shmptr)
{

	shmdt(shmptr);
	if(shmctl(shmid,IPC_RMID,NULL) < 0)
	{
		printf("shmctl failure:%s\n",strerror(errno));
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

void semaphore_exit(int semid)
{
	union semun semun_y;

	if(semctl(semid,0,IPC_RMID,semun_y) < 0)
	{
		printf("semctl end failure:%s\n",strerror(errno));
	}
	return;
}

