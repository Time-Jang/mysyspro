#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void findDirr(char *dirname, char *target, char *result)
{
	DIR* dirptr;
	struct dirent* dir;
	struct stat statbuf;
	char namebuf[256];
	int bufsize;
	strcpy(namebuf, dirname);
	bufsize = strlen(dirname);
	if((dirptr = opendir(dirname)) == NULL)
	{
		printf("There is no such directory");
		return;
	}
	namebuf[bufsize++] = '/';
	while((dir = readdir(dirptr)) != NULL)
	{
		strcpy(namebuf + bufsize, dir->d_name);
		if(stat(namebuf,&statbuf) < 0)
		{
			printf("There is no such directory");
			return;
		}
		if(!strcmp(dir->d_name,".") || !strcmp(dir->d_name,".."))
			continue;
		if(!strcmp(dir->d_name, target) && strcmp(target,"\0NULL"))
		{
			strcpy(result, namebuf);
			strcpy(target,"\0NULL");
		}
		if(S_ISDIR(statbuf.st_mode))
			findDirr(namebuf,target,result);
	}
	if(strcmp(target,"\0NULL"))
		strcpy(result,"NULL");
	closedir(dirptr);	
}

int findDir(char *dirname, char *target, char **find, int count)
{
	DIR* dirptr;
	struct dirent* dir;
	struct stat statbuf;
	char namebuf[256];
	int bufsize;
	strcpy(namebuf, dirname);
	bufsize = strlen(dirname);
	if((dirptr = opendir(dirname)) == NULL)
	{
		return 0;
	}
	namebuf[bufsize++] = '/';
	while((dir = readdir(dirptr)) != NULL)
	{
		strcpy(namebuf + bufsize, dir->d_name);
		if(stat(namebuf,&statbuf) < 0)
			return 0;
		if(!strcmp(dir->d_name,".") || !strcmp(dir->d_name,".."))
			continue;
		if(!strcmp(dir->d_name, target))
		{
			find[count] = (char *)malloc(sizeof(char) * (bufsize+5));
			strcpy(find[count], namebuf);
			count++;
		}
		if(S_ISDIR(statbuf.st_mode))
			count = findDir(namebuf,target,find,count);
	}
	closedir(dirptr);
	return count;
}

int main(int argc, char *argv[])
{
	char *dirname;
	int count = 0;
	int j = 0;	
	
	dirname = (char *)malloc(sizeof(char) * 64);
	getcwd(dirname, 64);
	if(argc < 1 || argc >2)
	{
		return 0;
	}
	if(argc == 2)
	{
		char *result;
		
		result = (char *)malloc(sizeof(char) * 512);
		findDirr(dirname, argv[1], result);
		chdir(result);
		if(execl("/bin/bash",NULL) < 0)
			exit(1);
		return 1;
	}
	if(argc == 1)
	{
		char **find;
		char input[16];
		
		find = (char **)malloc(sizeof(char *) * 20);
		printf(">Enter the name of a directory :");
		scanf("%s",input);	
	
		while(strcmp(input,"."))
		{	
			count = findDir(dirname,input,find,count);	
			if(count < 1)
			{
				printf("There is no such directory");
				break;
			}
			else if(count == 1)
			{
				printf("%s>",find[0]);
				scanf("%s",input);
				count = 0;
			}
			else
			{
				for(j = 0; j < count; j++)
					printf("[%d]	%s\n",(j+1),find[j]);
				printf("Which directory do you want ? ");
				scanf("%d",&j);
				printf("%s>",find[j-1]);
				strcpy(dirname,find[j-1]);
				scanf("%s",input);
				count = 0;
			}
		}
		printf("\n");
		for(j = 0; j < count;j ++)
			free(find[j]);
		free(find);
		return 1;
	}
}

