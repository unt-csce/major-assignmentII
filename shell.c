/*Group members: Loc Huynh, Christopher Hines, Peter Ogunrinde, & Edmund Sannda
 *Instructor: Mark Thompson		Class: CSCE3600		Due Date: 07/06/2015
 *Details: Major Assignment --- Unix/linux Shell
 */

//#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_SIZE 1024
#define MAX_COMMAND 20 /* can process up to 50 command at one time */
#define MAX_ARGUMENT 20 /* can process up to 15 arguments per command */

struct Command 
	{
	char * Name;				   /* Name of a command */
	char * Argument[MAX_ARGUMENT]; /* Array of arguments of this command */
	int    Type;                   /* 0: command without argument, 1: command has argumenrs, 2: special command: quit, exit, cd, ... */
	int    NumOfArgument;          /* hold a total argument of a command */
	};

void ProcessCommand(struct Command );
char *trimwhitespace(char *str);
void ParseCommand(struct Command list[MAX_COMMAND], int *, char * );
void ParseCommandArgument(struct Command list[MAX_COMMAND], int);
void ResetCommand(struct Command list[MAX_COMMAND], int);
void ProcessSpecialCommand(char *);

int main(int argc, char ** argv)
{
	struct Command lstCommand[MAX_COMMAND];   /* array to hold all commands from interactive or batch mode */
	int     numOfCommand;					  /* Num of commands */
	int j;
	int i;

	i = 0;
	j = 0;
	numOfCommand = 0;

	/* Initial list of commands */
	for (i = 0; i < MAX_COMMAND; i++)
		{
		lstCommand[i].Name = (char *) malloc(sizeof(char) * (MAX_SIZE + 1));
		for (j = 0; j < MAX_ARGUMENT; j++)
			{
			lstCommand[i].Argument[j] = (char *) malloc(sizeof(char) * (MAX_ARGUMENT + 1));
			}
		lstCommand[i].Type = 0; /* 0 is specific for a command without argument */
		lstCommand[i].NumOfArgument = 0;
		}

	if (argc == 1)
		{	

		while (1)
			{
			//interactive mode.
			//print out the prompt
			printf("prompt> ");

			//read command
			char * userInput;
			userInput = (char*)malloc(MAX_SIZE);		
			//read command from prompt
			fgets (userInput, MAX_SIZE, stdin);


			/* Remove trailing newline, if there. */
			if ((strlen(userInput)>0) && (userInput[strlen (userInput) - 1] == '\n'))
				userInput[strlen (userInput) - 1] = '\0';

			//parse the commands line from user input
			ParseCommand(lstCommand, &numOfCommand, userInput);

			//parse command argument from each commands.
			ParseCommandArgument(lstCommand, numOfCommand);

			//print out user commands and its arguments.
			for (i = 0; i < numOfCommand; i ++)
				{
				if (lstCommand[i].Type == 0) 
					{
					printf("Command without argument: %s\n", lstCommand[i].Name);
					}
				else if (lstCommand[i].Type == 1) 
					{
					printf("Command with argument: %s\n", lstCommand[i].Name);
					for (j = 0; j < lstCommand[i].NumOfArgument; j++)
						{
						printf("Argument: %s\n", lstCommand[i].Argument[j]);
						}
					}

				}
			
			for (i = 0; i < numOfCommand; i++)
				{
				/* process all commands here */
				printf("Process command: %s\n", lstCommand[i].Name);

				if (lstCommand[i].Type == 2) /* special command need to process difference from normal */
					{
					ProcessSpecialCommand(lstCommand[i].Name);
					}
				
				//delay();
				ProcessCommand(lstCommand[i]);
				}		


			 /* Free memory and exit. */
			free (userInput);
			ResetCommand(lstCommand, numOfCommand);
			numOfCommand = 0;
			}

		}
	else 
		{
		//batch mode.

		}
	
	return 0;
} /* main */

void ProcessCommand(struct Command cmd)
{
	while(1)
		{
			if(fork() == 0)
			{
				/* child */
			if (cmd.Type == 0) 
				{
				/* command without argument */
				execlp(cmd.Name, cmd.Name, (char *)0 );
				printf("execlp failed\n");
				}
			else if (cmd.Type == 1)
				{
				/* command has argument */
				char *execArgs[10]; // = { "echo", "Hello, World!", NULL };
				execArgs[0] = cmd.Name;
				printf("execArgs[0] = %s\n", execArgs[0]);

				int i;
				for (i = 0; i < cmd.NumOfArgument; i++)
					{
					execArgs[i + 1] = cmd.Argument[i];
					printf("execArgs :%s\n", execArgs[i + 1]);
					}

				execArgs[i + 1] = NULL;
				printf("execArgs[%d] = %s\n", i + 1, execArgs[i + 1]);

				execvp(execArgs[0], execArgs);

				printf("execvp failed\n");
				}
				
				exit(1);
			}
			else 
			{
				/* parent */
				wait( (int *)0 );
				printf("child finished\n");
				break;
			}
		}
}

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/* This function will parse all commands from a string, then save them to an array of all commands */
void ParseCommand(struct Command list[MAX_COMMAND], int * numOfCommand, char * input )
{
	char * pToken;
	pToken = strtok (input, ";"); /* each command seperated by semicolon ; character */
	//seperate command from user input
	if (pToken != NULL)
		{
		list[(*numOfCommand)].Name = trimwhitespace(pToken);

		/* handle special command here */
		if (strcmp(list[(*numOfCommand)].Name, "quit") == 0
			|| strcmp(list[(*numOfCommand)].Name, "exit") == 0
			|| strcmp(list[(*numOfCommand)].Name, "cd") == 0)
			{
			list[(*numOfCommand)].Type = 2; //special command
			}

		(*numOfCommand) ++;
		}
	do	
		{				
		pToken = strtok (NULL, ";");	// the NULL here means use the same line of characters we started with
		if (pToken == NULL)
			continue;
		list[(*numOfCommand)].Name = trimwhitespace(pToken);
		(*numOfCommand) ++;			
		} 
	while (pToken != NULL);
}

/* This function will parse all argument in each command, then update its command name if any */
void ParseCommandArgument(struct Command list[MAX_COMMAND], int numOfCommand)
{
	int i;
	for (i = 0; i < numOfCommand; i ++)
		{
		char * command = list[i].Name;

		int j;
		j = 0; /* number of argument of each command */
		char * pToken;
		pToken = strtok (command, " "); /* each argumenr seperated by space " " character */
		////seperate argument
		//if (pToken != NULL)
		//	{
		//	list[i].Argument[j] = trimwhitespace(pToken);
		//	j ++;
		//	}
		do	
			{				
			pToken = strtok (NULL, " ");	// the NULL here means use the same line of characters we started with
			if (pToken == NULL)
				continue;
			list[i].Argument[j] = trimwhitespace(pToken);
			j ++;			
			} 
		while (pToken != NULL);	

		if (j > 0) 
			{
			/* command has argument, need to update its command type. */
			list[i].Type = 1;
			list[i].NumOfArgument = j; /* update total argument */
			}
		}	
}

void ProcessSpecialCommand(char * commandName)
	{
	if (strcmp(commandName, "quit") == 0
		|| strcmp(commandName, "exit") == 0)
		{
		//quit the program.
		exit(0);
		}
	}

void ResetCommand(struct Command list[MAX_COMMAND], int numOfCommand)
	{
	int i,j;
	for (i = 0; i < numOfCommand; i++)
		{
		memset(list[i].Name, '\0', MAX_SIZE);
		for (j = 0; j < list[i].NumOfArgument; j ++)
			{
			memset(list[i].Argument[j], '\0', MAX_SIZE);
			}
		list[i].Type = 0;
		list[i].NumOfArgument = 0;
		}

	}



