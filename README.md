		The Shell and System Calls

Description:
------------

This is command line interpreter and a shell that operates in both
batch mode and interactive mode similar to a standard UNIX/Linux
bash shell.

The Latest Version
------------------
Details of the latest version can be found at our github project
page https://github.com/unt-csce/major-assignmentII


File List
---------
shell.c
bash
README.md
Makefile

Compiling the assignment
----------------------------
Simply upload all the file to your directory and run "make" to 
compile all the included files,in this case, we only have one
.c file. (no separate header files or .c are included)

Instructions
------------
The progrm starts by evaluating the number of argc received. If 
there is only 1 argument, then it will run on a interactive mode, 
otherwise if there are two arguments, it will run on a batch mode.  
	
	Interactive mode
	----------------
	We read the command from the prompt and process the user 
	input.

	Batch mode
	----------
	We open and get the file handle and check if file exist, 
	if it does, we read line by line and process input from 
	bash file.

We are processing input using an array that hold all the commands 
either in batch or interactive mode.

Design Overview
---------------
-	We have designed the Shell in four main sub functions to handle all commands from user (interactive mode) and from a file (batch mode). They are Process Input, Parse Commands, Parse Command Arguments, and then Process Command.

-	Process Input function: This function will read all input from user by user command line (interactive mode) or from a Batch File (batch mode). The output of this function is a string 

-	Parse Commands function: This function will take an input string from Process Input function, then parses this string into a separate commands. Each command has structure as below: 
struct Command 
	{
	Char *	Name;			    /* Name of a command */
	Char *	Argument[MAX_ARGUMENT];  /* Array of arguments of this command */
	CommandType     Type;	    /* NORMAL: command without argument, ARGUMENT:command has argumenrs, SPECIAL: special command: quit, exit, cd, ... */
	int	NumOfArgument;          /* hold a total argument of a command */
	};
	The output of this function is an array of Commands. Name of each command is parsed from the input string that is separate by a semicolon “;”. At this point, the command also contains many arguments from the input string, so we need to separate command argument from the command name.
	
-	Parse Command Argument: this function will take an array of Commands that is processed from Parse Command function, then find the arguments for each command by parsing the command name that is separated by a space ‘ ‘ characters. During parsing the argument for a command, this function also determine which command is normal (a command does not have any argument), arguments (a command has more than one argument), and special commands (quit, exit, history, prompt, cd, and CTRL-D). The output of this function is an array of command which contains a list of commands parsed from an input string. At this point, a command struct has been filled in (it means that shell already knows name of the command, how many arguments a command has, name of each argument, and the type of a command).

-	Process Command: this function is responsible to execute a command in a child process. The concurrency process happens in this function   
o	Input of this function is a command: the process for a command totally depends on its type. 
o	Normal command: use execlp to run
o	Argument command: use execvp to run
o	Special command: the shell will handle the special command in a special way: There are 6 special commands the shell can handle in this function, they are
     + Quit, exit, cd, CTRL+D, history, prompt

Complete Specification
-----------------------------	 
- Shell is using the strtok to parse a command from the input string that is separate by a semi-colon “;” into each token. After having a list of tokens, the shell will remove all space characters occurred at the beginning and ending of every token. As a result, if a token is a valid command if its length is greater than 0.
	+ As a result, the Shell can quickly remove any invalid command from the input string such as lines that have no commands between semi-colons, or a command that has many space character at the beginning or at the end, or many semi-colons in a command and so on. 

Known Bugs or Problems
----------------------------
- At this point, the shell is using perror function to display all error while running a command, so these error may cause some confuses from user perpective.
- On the prompt change it works when you include > on your change... i.e. prompt shellprompt> or prompt shellprompt$ and it works perfectly... so that part we can include on our readme file. So, its just the extra space.

OPTIONAL SHELL FUNCTIONALITY
----------------------------
 - Allow the user to customize the prompt, by typing 
      prompt newprompt
 - Add a shell history of previous commands run on the shell, by typing
	  history
 
	
Licensing
---------
None

Credits and Acknowledgements
----------------------------
Team: Loc Huynh
	tanlocmail@gmail.com
      Christopher Hines
	christopherhines2016@gmail.com
      Peter Ogunrinde
	pettoss@aol.com
      Edmund Sannda
        eksannda@gmail.com
