#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#define max_history 100
#define numberchars 200
#define TRUE 1
#define FALSE 0

char history_array[max_history][numberchars];
int history_index;

char **parse(char *buffer,char **commands,int *semicolons)
{
	int i,j,k,comma;
	for(i = 0, comma = 1; buffer[i] != '\0'; i++)
		if(buffer[i] == ',')
			comma = comma + 1;
	*semicolons = comma;
	commands = (char **)malloc(sizeof(char *)*comma);
	for(i = 0; i < comma; i++)
		commands[i] = (char *)malloc(sizeof(char)*numberchars);
	
	for(i = 0, j = 0, k = 0; buffer[i] != '\0'; i++)
	{
		if(buffer[i] == ',')
		{
			k = 0;
			j++;
		}
		else
		{
			commands[j][k] = buffer[i];
			k++;
		}
	}
	
	return commands;
}

int contain_and(char **commands)
{
	int i,j;
	for(i = 0; ; i++)
	{
		for(j = 0; ; j++)
		{
			if(commands[i][j] == '\n')
				return FALSE;
			if(commands [i][j] == '&')
				return TRUE;
		}
	}
		
	return FALSE;
}

int contain_redirection(char **commands) // 0 : no, 1 : >>, 2 : >!, 3 : >, 4 : <
{
	int i,j;
	for(i = 0; ; i++)
	{
		for(j = 0; ; j++)
		{
			if(commands[i][j] == '\n')
				return FALSE;
			if(commands[i][j] == '>')
			{
				if(commands[i][j+1] == '>')
					return 1;
				if(commands[i][j+1] == '!')
					return 2;
				return 3;
			}
			if(commands[i][j] == '<')
				return 4;
			
		}
	}
	return FALSE;
}

void redirection_file(char **commands, char *newfile, int *fmask)
{
	int i,j,redirect,k;
	for(i = 0; redirect == 0; i++)
	{
		for(j = 0; ; j++)
		{
			if(commands[i][j] == '>')
			{
				if(commands[i][j+1] == '>')
				{
					redirect = 1;
					break;
				}
				if(commands[i][j+1] == '!')
				{
					redirect = 2;
					break;
				}
				redirect = 3;
				break;
			}
			if(commands[i][j] == '<')
			{
				redirect =4;
				break;
			}
		}
	}
	if(redirect != 3)
		j++;
	i = i - 1;
	
	for(k = 1; commands[i][j+k] == ' ';k++)
		;
	j = j + k;
	
	for(k = 0; ; j++,k++)
	{
		if(commands[i][j] == '\0' || commands[i][j] == ' ' || commands[i][j] == '\n')
			break;
		newfile[k] = commands[i][j];
	}
	if(redirect == 4) // file input
		*fmask = 0666;
	else if(redirect == 3) // file ouput
		*fmask = 0666;
	else if(redirect == 2) // file create, file ouput
		*fmask = 0666;
	else if(redirect == 1) // append file ouput
		*fmask = 0666;
}

int contain_pipe(char **commands)
{
	int i,j;
	for(i = 0; ; i++)
	{
		for(j = 0; ; j++)
		{
			if(commands[i][j] == '\n')
				return FALSE;
			if(commands [i][j] == '|')
				return TRUE;
		}
	}
		
	return FALSE;
}

int contain_option(char *command)
{
	int i;
	for(i = 0;command[i] != '\0' ;i++)
	{
		if(command[i] == '\n')
			break;
		if(command[i] == '|')
			break;
		if(command[i] == '>')
			break;
		if(command[i] == '<')
			break;
		if(command[i] == '&')
			break;
		if(command[i] == '-')
			return TRUE;
		if(command[i] == ' ' && isalpha(command[i+1]))
			return TRUE;
		if(command[i] == '\'')
			return TRUE;
		if(command[i] == '.')
			return TRUE;
		if(command[i] == '"')
			return TRUE;
	}
	return FALSE;
}

char *parse_command_with_option(char **commands,int command_index,int pipe_cnt)
{
	char *command;
	int i,j,k,pipe_count;
	for(pipe_count = 0,i = 0; pipe_cnt != pipe_count ; i++)
	{
		if(commands[command_index][i] == '\0')
			break;
		if(commands[command_index][i] == '\n')
			break;
		if(commands[command_index][i] == '|')
			pipe_count = pipe_count + 1;
	}
	//해당 파이프가 있는 곳까지의 index를 i에 저장
	
	if(commands[command_index][i] == ' ')
	{
		for(i = i+1; commands[command_index][i] == ' '; i++)
			;
		//만약 파이프 뒤에 ' '가 있으면 글자까지 index 옮기기
	}
	
	for(j = 0; commands[command_index][i+j] != '\0'; j++)
	{
		if(commands[command_index][i+j] == '\n')
			break;
		if(commands[command_index][i+j] == '|')
			break;
	} // j = command와 option까지의 글자수
	
	command = (char*)malloc(sizeof(char)*(j+1));
	
	for(k = 0; k < j; k++)
		command[k] = commands[command_index][i+k];
	
	command[k] = '\0';
	return command;
}

char *parse_command(char *command_with_option)
{
	char *command;
	int i,j,k,len;
	
	len = strlen(command_with_option);
	
	command = (char*)malloc(sizeof(char)*(len+1));
	
	for(i = 0; command_with_option[i] != '\0' ;i++)
	{
		if(command_with_option[i] == ' ')
			break;
		command[i] = command_with_option[i];
	}
	command[i] = '\0';
	
	return command;
}

char **parse_option(char *command,int *option_size)
{
	int i,j,k,count_space;
	char **option;
	for(i = 0; command[i] != '\0'; i++) //명령어가 끝난 부분 탐색
		if(command[i] == ' ')
			break;

	for(; command[i] != '\0'; i++) //명령어 뒤로 오는 공백 넘기기
		if(command[i] != ' ')
			break;
	
	for(k = 0, count_space = 0; command[i+k] != '\0'; k++) // 글자 수와 옵션 수 얻기
	{
		if(command[i+k] == '&')
			break;
		if(command[i+k] == '>')
			break;
		if(command[i+k] == '<')
			break;
		
		if(command[i+k] == ' ')
			count_space++;
		if(command[i+k] == '\n')
			break;
		if(command[i+k] == '|')
			break;
	}
	*option_size = count_space + 1;
	
	option = (char**)malloc(sizeof(char*)*(count_space));
	option[0] = (char*)malloc(sizeof(char)*(numberchars));
	
	for(k = 0, j = 0; command[i] != '\0'; i++)
	{
		if(count_space < j)
			break;
		if(command[i] == '&')
		{
			i++;
			continue;
		}
		
		if(command[i] == ' ')
		{
			option[j][k] = '\0';
			k = 0;
			j++;
			option[j] = (char*)malloc(sizeof(char)*(numberchars));
			continue;
		}
		
		option[j][k] = command[i];
		k++;
	}
	
	return option;
}

int get_pipe_count(char *commands)
{
	int pipe_count,i;
	for(pipe_count = 0, i = 0; commands[i] != '\0'; i++)
	{
		if(commands[i] == '\n')
			break;
		if(commands[i] == '|')
			pipe_count++;
	}
	return pipe_count;
}

void history(char **commands, int semicolons)
{
	int i,j,k;
	if(history_index >= max_history)
	{
		for(i = 0, j = semicolons; j < max_history;i++,j++)
		{
			bzero(history_array[i],numberchars);
			for(k = 0; k < numberchars; k++)
			{
				if(history_array[j][k] == '\n')
					break;
				if(history_array[j][k] == '\0')
					break;
				history_array[i][k] = history_array[j][k];
			}
		}
		for(j = 0;j < semicolons; i++,j++)
		{
			bzero(history_array[i],numberchars);
			for(k = 0; k < numberchars; k++)
			{
				if(commands[j][k] == '\n')
					break;
				if(commands[j][k] == '\0')
					break;
				history_array[i][k] = commands[j][k];
			}
		}
	}
	else
	{
		for(i = 0; i < semicolons; i++)
		{
			for(j = 0; j < numberchars; j++)
			{
				if(commands[i][j] == '\n')
					break;
				if(commands[i][j] == '\0')
					break;
				history_array[history_index + i][j] = commands[i][j];
			}
		}
		history_index = history_index + semicolons;
	}
}

void history_command()
{
	int i;
	for(i = 0; i < history_index; i++)
		printf("%d. %s\n",(i+1),history_array[i]);
}

void ChangeDirectory(char *option)
{
	if(chdir(option))
		perror("cd error:");
}

char *recall(char *command) //$!command
{
	int i,j;
	char num[2];
	for(i = 0; i < 2; i++)
		num[i] = command[i+1];
	j = atoi(num) - 1;
	command = (char*)malloc(sizeof(char)*numberchars);
	for(i = 0; i < numberchars; i++)
		command[i] = history_array[j][i];
	return command;
}

void shell()
{
	int amp,redirect,fd,fmask,files[2],amper,reid,status,option_size,pipe_count,pipe_index,command_index,semicolons,i;
	char buffer[numberchars],**commands,newfile[numberchars],*command1,**option,pwd[255];
	semicolons = 0;
	
	getcwd(pwd,255);
	printf("%s$",pwd);
	while(fgets(buffer,numberchars,stdin)) //read(stdin, buffer, numberchars)
	{
		
		//parse command line
		for(i = 0; i < semicolons; i++)
			bzero(commands[i],numberchars);
		
		commands = parse(buffer,commands,&semicolons); //semicolons는 명령어의 수
		history(commands,semicolons);
//		printf("%s\n",commands[0]);
//		commands[0] = "ls";
		for(command_index = 0; command_index < semicolons; command_index++)
		{		
//			printf("%s\n",commands[command_index]);
			if(contain_and(commands)) //command line contains &?
				amp = 1;
			else
				amp = 0;
	
			if(fork() == 0)
			{
				if(contain_redirection(commands) != FALSE) //redirect output
				{
					redirection_file(commands,newfile,&fmask);
					fd = creat(newfile, fmask);
					fclose(stdout);
					dup(fd);
					close(fd);
				}
				if(contain_pipe(commands) != FALSE) //piping
				{
					pipe_count = get_pipe_count(commands[command_index]);
					pipe(files);
					for(pipe_index = pipe_count; pipe_index > 0; pipe_index--)
					{
						if(fork() == 0)
						{
							command1 = parse_command_with_option(commands,command_index,pipe_index);
							//first component of command line
							fclose(stdout);
							dup(files[1]);
							close(files[1]);
							close(files[0]);
							//stdout now goes to pipe
							//child process does command1
							if(command1[0] == '!')
								command1 = recall(command1);
							if(strcmp(command1,"history") == 0)
								history_command();
							else if(strcmp(command1,"exit") == 0)
								kill(0,SIGKILL);
							else if(contain_option(command1))
							{
								option = parse_option(command1,&option_size);
								command1 = parse_command(command1);
								if(option_size == 1)
								{
									if(strcmp(command1,"cd") == 0)
										ChangeDirectory(option[0]);
									else
										execlp(command1,command1,option[0],0);
								}
								else if(option_size == 2)
									execlp(command1,command1,option[0],option[1],0);
								else if(option_size == 3)
									execlp(command1,command1,option[0],option[1],option[2],0);
								else if(option_size == 4)
									execlp(command1,command1,option[0],option[1],option[2],option[3],0);
							}
							else
							{
								command1 = parse_command(command1);
								execlp(command1, command1, 0);
							}
						}
						command1 = parse_command_with_option(commands,command_index,pipe_index - 1);
						//2nd command of command line
						fclose(stdin);
						dup(files[0]);
						close(files[0]);
						close(files[1]);
						//stdin now comes from pipe 
					}
				}
				command1 = parse_command_with_option(commands,command_index,0);
//				printf("command1 : %s\n",command1);

				
				if(command1[0] == '!')
					command1 = recall(command1);
				if(strcmp(command1,"history") == 0)
					history_command();
				else if(strcmp(command1,"exit") == 0)
					kill(0,SIGKILL);
				else if(contain_option(command1))
				{
					option = parse_option(command1,&option_size);
					command1 = parse_command(command1);
					
					if(option_size == 1)
					{
						if(strcmp(command1,"cd") == 0)
							ChangeDirectory(option[0]);
						else
							execlp(command1,command1,option[0],0);
					}
					else if(option_size == 2)
						execlp(command1,command1,option[0],option[1],0);
					else if(option_size == 3)
						execlp(command1,command1,option[0],option[1],option[2],0);
					else if(option_size == 4)
						execlp(command1,command1,option[0],option[1],option[2],option[3],0);
				}
				else
				{
					command1 = parse_command(command1);
					execlp(command1, command1, 0);
				}
			}
			//parent continues over here,
			//wait for child to exit if required
			
			if(amp == 0)
				reid = wait(&status);

			getcwd(pwd,255);
			printf("%s$",pwd);
		}
	}
}


int main(int argc, char *argv[])
{
	history_index = 0;
	shell();
}