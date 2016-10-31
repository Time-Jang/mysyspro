#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
void error(char *text)
{
	perror(text);
	exit();
}
DIR *findDir(char *target)
{
	DIR* tar_ptr;
	struct dirent* tar_ent;
	if((tar_ptr = opendir(target)) == NULL)
	{
		error("opendir error");
	}
	while((tar_ent = readdir(tar_ptr)) == NULL)
	{
		if(
	}
}
int main(int argc, char *argv)
{
	if(argc < 1)
	{
		error("Input Directory Name");		
	}	

}
