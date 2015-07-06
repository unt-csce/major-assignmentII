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
typedef enum {NORMAL, ARGUMENT, SPECIAL} CommandType;

struct Command 
	{
	char		*	Name;				    /* Name of a command */
	char		*	Argument[MAX_ARGUMENT]; /* Array of arguments of this command */
	CommandType     Type;					/* NORMAL: command without argument, ARGUMENT: command has argumenrs, SPECIAL: special command: quit, exit, cd, ... */
	int				NumOfArgument;          /* hold a total argument of a command */
	};

void ProcessCommand(struct Command );
char *trimwhitespace(char *str);
void ParseCommand(struct Command list[MAX_COMMAND], int *, char * );
void ParseCommandArgument(struct Command list[MAX_COMMAND], int);
void ResetCommand(struct Command list[MAX_COMMAND], int);
void ProcessSpecialCommand(struct Command );
void ProcessInput(char *);
void ProcessInteractiveMode();
int  ProcessBatchMode(const char * fileName);

int main(int argc, char ** argv)
{
	if (argc == 1) /* Interactive Mode */
		{	
		ProcessInteractiveMode();
		}
	else if (argc == 2)   /* Batch Mode */
		{
		//batch mode.
		ProcessBatchMode(argv[1]);			
		}
	
	return 0;
} /* main */

/* Process input from file (bash mode) or user (interactive mode) */
void ProcessInput(char * input)
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
		lstCommand[i].Type = NORMAL;		/* NORMAL is specific for a command without argument */
		lstCommand[i].NumOfArgument = 0;
		}

	//parse the commands line from user input
	ParseCommand(lstCommand, &numOfCommand, input);

	//parse command argument from each commands.
	ParseCommandArgument(lstCommand, numOfCommand);

	//print out user commands and its arguments.
	for (i = 0; i < numOfCommand; i ++)
		{
		if (lstCommand[i].Type == NORMAL) 
			{
			printf("Command without argument: %s\n", lstCommand[i].Name);
			}
		else if (lstCommand[i].Type == ARGUMENT) 
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

		if (lstCommand[i].Type == SPECIAL) /* special command need to process difference from normal */
			{
			ProcessSpecialCommand(lstCommand[i]);
			continue; //continue process other command if any
			}
				
		//delay();
		ProcessCommand(lstCommand[i]);
		}		

	//reset command struct, ready for next input
	ResetCommand(lstCommand, numOfCommand);
	numOfCommand = 0;
	}

/* This function will run each command in a child process */
void ProcessCommand(struct Command cmd)
{
	while(1)
		{
			if(fork() == 0)
			{
				/* child */
			if (cmd.Type == NORMAL) 
				{
				/* command without argument */
				execlp(cmd.Name, cmd.Name, (char *)0 );
				//printf("execlp failed\n");
				perror ("The following error occurred:\n");
				}
			else if (cmd.Type == ARGUMENT)
				{
				/* command has argument */
				char *execArgs[10]; // = { "echo", "Hello, World!", NULL };
				execArgs[0] = cmd.Name;
				//printf("execArgs[0] = %s\n", execArgs[0]);

				int i;
				for (i = 0; i < cmd.NumOfArgument; i++)
					{
					execArgs[i + 1] = cmd.Argument[i];
					//printf("execArgs :%s\n", execArgs[i + 1]);
					}

				execArgs[i + 1] = NULL;
				//printf("execArgs[%d] = %s\n", i + 1, execArgs[i + 1]);

				execvp(execArgs[0], execArgs);

				//printf("execvp failed\n");
				perror ("The following error occurred:\n");
				}
				
				exit(1);
			}
			else 
			{
				/* parent */
				wait( (int *)0 );
				//printf("child finished\n");
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
			list[(*numOfCommand)].Type = SPECIAL; //special command
			}

		(*numOfCommand) ++;
		}
	do	
		{				
		pToken = strtok (NULL, ";");			// the NULL here means use the same line of characters we started with
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
			if (list[i].Type == NORMAL)
				list[i].Type = ARGUMENT;

			printf("list[i].Name = %s\n", list[i].Name);
			/* process for cd command */
			if (strcmp(list[i].Name, "cd") == 0)
				{
				list[i].Type = SPECIAL;
				}

			list[i].NumOfArgument = j; /* update total argument */
			}
		}	
}

void ProcessSpecialCommand(struct Command cmd)
	{
	if (strcmp(cmd.Name, "quit") == 0
		|| strcmp(cmd.Name, "exit") == 0)
		{
		//quit the program.
		exit(0);
		}
	else if (strcmp(cmd.Name, "cd") == 0)
		{
		//process cd arguments command.	
		int ret;
		char buf[50];
		if (cmd.NumOfArgument > 0)
			{
			//sprintf(buf, "chdir %s", cmd.Argument[0]);
			ret = chdir (cmd.Argument[0]);
			getcwd(cmd.Argument[0],sizeof(cmd.Argument[0]));
			}
		else if (cmd.NumOfArgument == 0)
			{
			ret = chdir (" ");
			getcwd(" ", 1);
			}
		if(ret!=0)
			{
			perror("Error while process the cd command: ");
		  	}	
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
		list[i].Type = NORMAL;
		list[i].NumOfArgument = 0;
		}
	}

/* Handle shell in interactive mode */
void ProcessInteractiveMode()
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

		/* process for CTRL-D */
		if (userInput == NULL)
		{
		char c;
		printf("Write: ");
		scanf("%c", &c);
		if(c == 0x04)
			{
			printf("ctrl+D was entered\n");
			return;
			}
		}


		/* Remove trailing newline, if there. */
		if ((strlen(userInput)>0) && (userInput[strlen (userInput) - 1] == '\n'))
			userInput[strlen (userInput) - 1] = '\0';

		/* Process input from user */
		ProcessInput(userInput);						
		}
	}

/* Handle shell in Batch Mode */
int ProcessBatchMode(const char * fileName)
	{
	//open and get the file handle
	FILE* fh;
	fh = fopen(fileName, "r");

	//check if file exists
	if (fh == NULL){
		printf("File: %s does not exist!!!\n", fileName);
		return 0;
	}

	//read line by line
	const size_t line_size = 300;
	char* line = malloc(line_size);
	while (fgets(line, line_size, fh) != NULL)
		{
				
		/* Remove trailing newline, if there. */
		if ((strlen(line)>0) && (line[strlen (line) - 1] == '\n'))
			line[strlen (line) - 1] = '\0';

		/* Print command line from bash file */
		printf("Reading commands Line: %s\n", line);

		/* Process input from bash file */
		ProcessInput(line);
		}
	}

