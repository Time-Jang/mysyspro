#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

void myalarm()
{
	printf("ding dong dang\n");
}

void newalarm(int secs)
{
	int pid,status;
	pid = fork();
	if(pid == 0)
	{
		sleep(secs);
		kill(getppid(), SIGALRM);
		exit(0);
	}
	else
		return;
}

int main()
{
	int i = 0;

	printf("alarm setting\n");
	signal(SIGALRM, myalarm);
	newalarm(1);
	while(i < 5)
	{
		printf("ok\n");
		pause();
		newalarm(2);
		i++;
	}
	
}
