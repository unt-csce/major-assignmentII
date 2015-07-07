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
