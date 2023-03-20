#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
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



int main(int argc,char **argv)
{
	key_t   key;
	int     shmid;
	int     semid;
	int     i;
	char    *ptr=NULL;

	if((semid=semaphore_init()) < 0)
	{
		printf("semaphor_init error\n");
		goto cleanup;
	}
	key=ftok(PATH_NAME,PROJ_ID);
	if(key < 0)
	{
		perror("ftok error");
		return -1;
	}
	shmid=shmget(key,4096,IPC_CREAT|0666);
	printf("shmid: %d\n",shmid);
	if(shmid < 0)
	{
		perror("shmget error");
		return -2;
	}

	ptr=(char *)shmat(shmid,NULL,0);
	printf("ptr:%p\n",&ptr);

	semop_v(semid);

	*ptr='a'-1;
	printf("valure:%d\n",'z'-'a');
	for(i=0;i<'z'-'a';i++)
	{
		*ptr=*ptr+1;
		printf("ptr:%s\n",ptr);
		sleep(1);
	}
	*ptr='\0';

cleanup:
	semaphore_exit(semid);
	shmdt(ptr);
	return 0;
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
