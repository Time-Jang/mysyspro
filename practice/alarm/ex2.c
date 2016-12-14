#include <unistd.h>
#include <signal.h>
#include <stdio.h>

void myalarm()
{
	printf("ding dong dang\n");
}
int main()
{
	int i = 0;
	printf("alarm setting\n");
	signal(SIGALRM, myalarm);
	alarm(1);
	while(i<5)
	{
		printf("ok\n");
		pause();
		alarm(2);
		i++;
	}
}
