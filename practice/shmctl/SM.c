#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

void del_shm(int shmid)
{
	shmctl(shmid,IPC_RMID,0);
	exit(1);
}

void shm_p()
{
	int spid,status;
	spid = wait(&status);
	if(spid == -1)
	{
		perror("Child process killed not properly");
		exit(0);
	}
	printf("\n");
	exit(1);
}

int main()
{
	static struct sigaction act;
	int shmid, pid, *cal_num;
	void *shared_memory = (void *)0;
	
	
	shmid = shmget((key_t)1234,sizeof(int),0666|IPC_CREAT);
	if(shmid == -1)
	{
		perror("shmget failed: ");
		exit(0);
	}
	
	shared_memory = shmat(shmid,(void *)0,0);
	if(shared_memory == (void *)-1)
	{
		perror("shmat failed: ");
		exit(0);
	}
	cal_num = (int *)shared_memory;
	if((pid = fork()) == 0)
	{
		act.sa_handler = del_shm;
		sigfillset(&(act.sa_mask));
		sigaction(SIGINT,&act,NULL);
		*cal_num = 1;
		while(1)
		{
			*cal_num = *cal_num + 1;
			printf("[CHILD]%d\n",*cal_num);
			sleep(1);
		}
	}
	else if(pid > 0)
	{
		act.sa_handler = shm_p;
		sigfillset(&(act.sa_mask));
		sigaction(SIGINT,&act,NULL);
//		signal(SIGINT,SIG_IGN);
		while(1)
		{
			sleep(1);
			printf("[PARENT]%d\n",*cal_num);
		}
	}
}


//shmctl(shmid,IPC_RMID,0);