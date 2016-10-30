#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void print_error(char *text)
{
	perror(text);
	exit(1);
}
void print_tab(int i)
{
	int j;
	for(j = 0; j < i; j++)
		printf("     ");
}
void myftw(const char *dirname,int tab_num)
{	
	DIR* dirptr;
	struct dirent* dir;
       	struct stat statbuf;
	char namebuf[256];
	int buflen;
	strcpy(namebuf, dirname);	
	buflen = strlen(dirname);
	if((dirptr = opendir(dirname)) == NULL)
		print_error("Open directory is failed");
	namebuf[buflen++] = '/';
	print_tab(tab_num);
	printf("%s\n",dirname);
	while((dir = readdir(dirptr)) != NULL)
	{
		strcpy(namebuf + buflen, dir->d_name);
		if(stat(namebuf,&statbuf) < 0)
			print_error("lstat_error");
		if(!strcmp(dir->d_name,".") || !strcmp(dir->d_name,".."))
			continue;
		if(S_ISDIR(statbuf.st_mode))
			myftw(namebuf,tab_num+1);
		else
		{
			print_tab(tab_num + 1);
			printf("%s\n",namebuf);	
		}
	}
	closedir(dirptr);
}


int main(int argc, char **argv)
{
	myftw(argv[1], 0);
	return 0;
}
