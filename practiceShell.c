#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>

extern int errno;

//this is a test
typedef void (*sighandler_t)(int);

//arguments and environmental variables
static char *my_argv[100], *my_envp[100];

//path to search for programs
static char *search_path[10];

FILE *loadfile;
long double start[4], current[4], avg;

void dosuperbash(char *path);
void dowhoami();

void handle_signal(int signo)
{
	//prints out MY_SHELL
    printf("\n[MY_SHELL ] ");
    
    //not really necessary, but good practice
    fflush(stdout);
}

void fill_argv(char *tmp_argv)
{
	//pointer to list of arguments
    char *arg = tmp_argv;
    
    //counter
    int index = 0;
    
    char ret[100];
    
    //set ret to 0 for good measure
    memset(ret, '\0', sizeof(ret));
    
    //while we don't have a string terminator
    //take the tmp_argv and examine it character by character
    while(*arg != '\0') 
    { 
        
        //looks like we only support 10 arguments
        if(index == 10)
        {
            break;
		}
		
		//we found a space
        if(*arg == ' ') 
        {
			//if the index spot in my_argv isn't being used
            if(my_argv[index] == NULL)
            {
				//allocate enough memory for the index to store the argument
                my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			}
            else 
            {
				//set the index contents to a bunch of 0's
                memset(my_argv[index], '\0', strlen(my_argv[index]));
            }
            
            //now that we have allocated enough space
            //copy the ret to the spot in my_argv
            strncpy(my_argv[index], ret, strlen(ret));
            
            //add a string terminator for formatting purposes
            //strncat(my_argv[index], "\0", 1);
            
            //set memset back to a bunch of 0's
            memset(ret, '\0', 100);
            
            //rinse and repeat
            index++;
            
        } 
        else 
        {
			//not a space
			//so just copy it
            strncat(ret, arg, 1);
        }
        
        //increment to the next argument
        arg++;       
    }
    
    //set the arg index to the size it needs to be
    my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
    
    //copy ret to my_argv[index]
    strncpy(my_argv[index], ret, strlen(ret));
    
    //add a terminator at the end
    strncat(my_argv[index], "\0", 1);
}

//copy the environmental variables for use
void copy_envp(char **envp)
{
	//counter
    int index = 0;
    
    //increment through until envp[index] = NULL
    for(index = 0; envp[index] != NULL; index++) 
    {
		//cast our array index of environmental variables to a character pointer
		//with the proper memory allocated to hold the envp[index] + 1
        my_envp[index] = (char *) malloc(sizeof(char) * (strlen(envp[index]) + 2));
        
        //add a $ before each variabel name
        //PATH becomes $PATH
        strcat(my_envp[index], "$");
        
        //copy from envp to my_envp
        memcpy(my_envp[index], envp[index], strlen(envp[index]));
    }
}

//get the $PATH variable so we know where to look
//when we are trying to execute commands
void get_path_string(char **tmp_envp, char *bin_path)
{
	//counter
    int count = 0;
    char *tmp;
    while(1) 
    {
		//increment through the environmental variables
		
		//look for the $PATH one
        tmp = strstr(tmp_envp[count], "PATH");
        
        //keep incrementing until we find it
        if(tmp == NULL || strstr(tmp_envp[count], "MOZ") != NULL) 
        {
            count++;
        } 
        
        //we found it, so no need to keep looping
        else 
        {
				break;
        }
    }
		//copy the value of the $PATH variable to bin_path
        strncpy(bin_path, tmp, strlen(tmp));
}	

void insert_path_str_to_search(char *path_str) 
{
    //counter
    int index=0;
    
    //temporary pointer so we don't overwrite *path_str
    char *tmp = path_str;
    
    //array
    char ret[100];

	//set tmp to just past the = of PATH=/usr/bin
    while(*tmp != '=')
    {    
        tmp++;
    }
    
    //increment again, because tmp is at the =, we want to be after it
    tmp++;


	//while the character isn't a string terminator
    while(*tmp != '\0') 
    {
		//if the character is a :
		//the $PATH variable separates entries by a :
		//usr/bin:/usr/sbin etc
        if(*tmp == ':') 
        {
			//add a / to the spot in ret
			//to make it /bin/foo instead of bin/foo
            strncat(ret, "/", 1);
            
            //set the proper size for the path
            search_path[index] = (char *) malloc(sizeof(char) * (strlen(ret) + 1));
            
            //copy it
            strncat(search_path[index], ret, strlen(ret));
            
            //terminate the string
            strncat(search_path[index], "\0", 1);
            
            //increment
            index++;
            
            //reset ret back to terminators
            memset(ret, '\0', 100);
        } 
        else 
        {
			//copy it
            strncat(ret, tmp, 1);
        }
        //increment again to get to the next one
        tmp++;
    }
}
//takes the command typed (echo) and adds it file path
//so 'echo' becomes '/usr/bin/echo'
int attach_path(char *cmd)
{
	//ret again
    char ret[100];
    
    //counter
    int index;
    int fd;
    
    //set ret to \0's
    memset(ret, '\0', 100);
    
    //search all the search paths for the command given
    for(index = 0; search_path[index] != NULL; index++) 
    {
		//takes ret and adds the first search path entry to it
		//e.g /usr/bin
        strcpy(ret, search_path[index]);
        
        //appends the command given to the search path
        //e.g /usr/bin now becomes /usr/bin/echo
        strncat(ret, cmd, strlen(cmd));
        
        //check to see if /usr/bin/echo is a valid file
        //by trying to open the file in a read-only mode
        if((fd = open(ret, O_RDONLY)) > 0) 
        {
			//if it is, copy it to cmd
            strncpy(cmd, ret, strlen(ret));
            
            //don't leave the file open
            close(fd);
            return 0;
        }
        //else the file doesn't exist, which means it isn't a valid
        //command
    }
    //since we are just copying it to cmd, we don't need to return anything
    return 0;
}

//actually execute the program
void call_execve(char *cmd)
{
    int i; //return value for execve
    int counter;
    int subcounter;
    int noexecute = 0; //noexecute flag
        
    char *tmp;
	char *buffer;
    
    //print the command for debug
    //printf("cmd is %s\n", cmd);
    
    //increment through the arguments provided by a user
    for(counter = 0; my_argv[counter] != NULL; counter++)
    {
		//search for any environmental variables prefixed by a $ ($PATH, $USER, etc)
		if(strchr(my_argv[counter], '$') != NULL)
		{
			//we found one, so lets dissect it

	
			//tokenize it, and get rid of the $ and =, which leaves us with the name
			//of the variable.  e.g it goes from "$TERM" to just "TERM="
			tmp = strtok(my_argv[counter], "$");
			tmp = strcat(tmp, "=");
			
			//search the array of environmental variables for something that matched TERM=
			for(subcounter = 0; my_envp[subcounter] != NULL; subcounter++)
			{
			
				//if we found one and the sizes of the string sort of match
				//(the strcmp() check prevents stuff like PATH= being evaluated as MOZ_PLUGINS_PATH=)
				if(strstr(my_envp[subcounter], tmp) != NULL && strcmp(my_envp[subcounter], tmp) > 0)
				{	
					//get rid of the variable name and the =
					buffer = strtok(my_envp[subcounter], "=");
					buffer = strtok(NULL, my_envp[subcounter]);
					
					//buffer is now just the evaluated value of the variable
					my_argv[counter] = buffer;
					
					//reassign the environmental variable array index to its original value
					//somewhere along the lines I ended up setting it to overwrite it
					//which fucks up my search, so I just added this hack to fix it
					my_envp[subcounter] = strcat(tmp, buffer);
					
					//all done, so lets move on to the next possible environmental variable in
					//the argument array
					//break;
				}
			}
		}
		
		
		//file I/O redirection stuff
		int size, nullLocation;
		FILE *newfile;
		FILE *inputfile;
		char buff[255], buff1[255];
		
		//check for output redirection
		if(strchr(my_argv[counter], '>') != NULL)
		{
			
			//make sure > isn't by itself
			if(my_argv[counter - 1] != NULL && my_argv[counter + 1] != NULL)
			{
				//create stream to be read into a string called buff
				loadfile = popen(my_argv[counter - 1], "r");
				//get size of stream
				size = ftell(loadfile)+1;
				//sets nullLocation to # of elements read in
				nullLocation = fread(buff, 1, size-1, loadfile);
				//add null teminator to the end (fixed garbage)
				buff[nullLocation] = '\0';
				pclose(loadfile);
				
				//create a file with specified name and put buff(command output) into it
				newfile = fopen(my_argv[counter + 1], "w");
				fputs(buff,newfile);
				fclose(newfile);
				
				//clear out buff to prevent junk
				memset(buff, '\0', 100);
			}
			//don't execute
	    	noexecute = 1;
		}
		
		//check for input redirection
		if(strchr(my_argv[counter], '<') != NULL)
		{
			if(my_argv[counter - 1] != NULL && my_argv[counter + 1] != NULL)
			{
				//get input file data
				inputfile = fopen(my_argv[counter + 1], "r");
				//reads in data from file line by line until end of file
				while(fgets(buff1, 255, inputfile) != NULL)
				{
					strcat(buff, buff1);
				}
				fclose(inputfile);
				memset(buff1, '\0', 100);
				
				//put data into the command
				loadfile = popen(my_argv[counter - 1], "w");
				fputs(buff, loadfile);
				pclose(loadfile);

				memset(buff, '\0', 100);
			}
			
			noexecute = 1;
		}
		
		//our UNIX utility we chose is 'whoami'
		//so we made our own called 'mywhoami'
		if(strstr(my_argv[counter], "mywhoami") != NULL && counter == 0)
		{
			dowhoami();
			noexecute = 1;
		}
		
		//syntax checking for it
		if(strstr(my_argv[counter], "mywhoami") != NULL && counter != 0)
		{
			printf("format for command is 'mywhoami'\n");
			noexecute = 1;
		}
		
		//if there is an ampersand and it is the last argument, with one preceding it
		if(strchr(my_argv[counter], '&') != NULL && my_argv[counter + 1] == NULL && my_argv[counter - 1] != NULL)
		{
			//fork the command given by counter - 1
			//printf("& detected, you should fork %s.\n", my_argv[counter - 1]);
		 pid_t process;
                 process = fork();
                 int jb=0;
                 int status;
                 if(process< 0)
                 {
                 printf("fork failed");
                 }
                 if(process > 0)
                 {
                 //parent process
                 wait(&status);
                 }
    
                 if(process == 0)
                {//child process
                 //when & is read, forks, and job counter ++
                 //forks counter-1
                 jb++;
                 printf("\n[%d]\n", jb);
                 // printf("\nChild Process Executed");
                 printf("%s\n", my_argv[counter-1]);
                 }
                 return 0 ;
		}
		
		//if the first argument is superbash
		if(strstr(my_argv[0], "superbash") != NULL && my_argv[counter + 1] != NULL)
		{
			//send the file path to the dosuperbash function
			dosuperbash(my_argv[counter+1]);
			
			//don't try to execute the command
			noexecute = 1;
			
			//its a parser, so break
			break;
		}
		
		if(strstr(my_argv[counter], "cpuload") != NULL && counter == 0 && my_argv[counter + 1] == NULL)
		{
			//open it again to get current reading
			loadfile = fopen("/proc/stat","r");
			
			//scan them into the current array
			fscanf(loadfile,"%*s %Lf %Lf %Lf %Lf",&current[0],&current[1],&current[2],&current[3]);
			
			//close
			fclose(loadfile);
			
			//find the average (current + start / current - start) * 100
			avg = (((current[0] + current[1] + current[2]) - (start[0] + start[1] + start[2])) /
					((current[0] + current[1] + current[2] + current[3] ) - (start[0] + start[1] + start[2] + start[3]))) * 100;
					
			//print it
			printf("CPU load average since shell execution: %Lf\n", avg);
			
			//dont execute it and break
			noexecute = 1;
			break;
		}
		
		if(strstr(my_argv[counter], "cpuload") != NULL && counter != 0)
		{
			//syntax error handling
			printf("cpuload syntax: cpuload\n");
		}
		
		//to make sure they use the command format right
		if(strstr(my_argv[counter], "superbash") != NULL && counter != 0)
		{
			//superbash error handling
			printf("Superbash parser syntax: superbash <filename>\n");
		}
	}
    
    //if we aren't a child
    if(fork() == 0 && noexecute == 0) 
    {
		//try to execute the command
        i = execve(cmd, my_argv, my_envp);
        
        //print the error code
        printf("errno is %d\n", errno);
        
        //if the execve didn't execute properly
        if(i < 0) 
        {
			//command not found
            printf("%s: %s\n", cmd, "command not found");
            exit(1);        
        }
    } 
    else 
    {
		//we are a child, so wait
        wait(NULL);
    }
}


//frees memory allocated so memory leaks don't happen
void free_argv()
{
	//counter
    int index;
    
    //increment through my_argv array and set it to \0, then NULL
    for(index = 0; my_argv[index] != NULL; index++) 
    {
        memset(my_argv[index], '\0', strlen(my_argv[index]) + 1);
        my_argv[index] = NULL;
        
        //release it into memory now that it is clean
        free(my_argv[index]);
    }
}

//main function
int main(int argc, char *argv[], char *envp[])
{
    char c;
    int i, fd;
    char *tmp = (char *)malloc(sizeof(char) * 100);
    char *path_str = (char *)malloc(sizeof(char) * 256);
    char *cmd = (char *)malloc(sizeof(char) * 100);
    
    signal(SIGINT, SIG_IGN);
    signal(SIGINT, handle_signal);
    
    //open /proc/stat to get base reading of cpu usage
    loadfile = fopen("/proc/stat", "r");
    
    //record values to use in average
    fscanf(loadfile, "%*s %Lf %Lf %Lf %Lf",&start[0],&start[1],&start[2],&start[3]);
    
    //close file
    fclose(loadfile);

	//copy environmental variables given envp[]
    copy_envp(envp);

	//get the $PATH variable
    get_path_string(my_envp, path_str);
    
    //format the $PATH variable so we can use it
    //to search for commands   
    insert_path_str_to_search(path_str);

    //if we aren't a child
    if(fork() == 0) 
    {
        //execve("/usr/bin/clear", argv, my_envp);
        exit(1);
    } 
    
    //we are a child
    else 
    {
		//wait
        wait(NULL);
    }
    
    //print out the prompt
    printf("[MY_SHELL ] ");
    fflush(stdout);

	//c is where we store the input
    while(c != EOF) 
    {
		//get the input and send it through the switch case
        c = getchar();
        switch(c) 
        {
			//new line
            case '\n': if(tmp[0] == '\0') //if its the very beginning
						{
							//print the prompt
							printf("[MY_SHELL ] ");
						}
        
						else 
						{
							//fill the array with stuff from tmp
							fill_argv(tmp);
							
							//copy the command from my_argv to cmd
							strncpy(cmd, my_argv[0], strlen(my_argv[0]));
							
							//add a terminator
							strncat(cmd, "\0", 1);
							
							//if we specify a relative path vs an absolute
							//e.g echo vs /usr/bin/echo
							if(index(cmd, '/') == NULL) 
							{
								//attach the full file path to the cmd
								if(attach_path(cmd) == 0)
								{
									//then execute it if it returns sucessfully
									call_execve(cmd);
								}
                            
								else 
								{
									//else we can't find it
									printf("%s: command not found\n", cmd);
								}
							}
                       
							//else check the current directory for the command
							else 
							{
								//if we found it in the directory
								if((fd = open(cmd, O_RDONLY)) > 0) 
								{
									//close the file we opened
									close(fd);
									
									//execute the file
									call_execve(cmd);
								} 
                           
								//else it isn't in the current directory either
								else 
								{
									//not found
									printf("%s: command not found\n", cmd);
								}
							}
							
							//free up the memory
							free_argv();
							
							//print prompt
							printf("[MY_SHELL ] ");
							
							//set the cmd to nulls for the next command
							memset(cmd, '\0', 100);
						}
                   
						//set tmp to null so we don't have leftovers
						memset(tmp, '\0', 100);
						
						//break
						break;
                   
			default: strncat(tmp, &c, 1); //no return, so keep grabbing the characters
					 break;
        }
    }
    
    //free  all the memory
    free(tmp);
    free(path_str);
    
    for(i = 0; my_envp[i] != NULL; i++)
    {
        free(my_envp[i]);
    }
    
    for(i = 0; i < 10; i++)
    {
        free(search_path[i]);
    }
    
    printf("\n");
    return 0;
}

//superbash implementation
void dosuperbash(char *path)
{
	//file pointer
    FILE *file;
    
    //variable to parse script variables into
    char* varname;
	int varvalue;

    //we use this to read line by line
    char line[128];
    
    //attempt to open the file
    file=fopen(path,"r");

    if(file != NULL)
    {
        //we can open the file
        while(fgets(line, sizeof(line), file) != NULL)
        {
			//start the long list of conditionals
			
			//this handles the num=5 and other possible variable assignments
            if(strchr(line,'=') != NULL) 
            {
					//token for when I tokenize the string
					char* token;
				
					//line = num = 5
				
					//variable name
					token = strtok(line, " "); // num
					//save the name so I can spit it out later
					varname = token;
				
					//bypass the = sign
					token = strtok(NULL, " "); // =
				
					//hop over to what the variable is assigned to
					token = strtok(NULL, " "); // 5
					//cast it to an integer (probably not safe, but I don't think it matters on this assignment)
					varvalue = atoi(token);
					
					//print in valid bash syntax
					printf("%s=%i\n\n",varname,varvalue);
			}
			
			//handle if statement logic
            if(strstr(line,"if") != NULL)
            {
					//more character arrays
					char* conditional;
					char buffer[128];
					
					//separate the line by the $
					conditional = strtok(line, "$");
					
					//skip over one increment and stash it in buffer
					conditional = strtok(NULL, line);
					strcat(buffer,conditional);
					
					//append a space to make it valid syntax
					strcat(buffer," ");
					
					//more parsing and grabbing
					conditional = strtok(NULL, line);
					strcat(buffer,conditional);
					strcat(buffer," ");
					
					conditional = strtok(NULL, line);
					strcat(buffer,conditional);
					
					//print it out in valid format as well
					//as a correpsonding "then" on the following line
					printf("if[ $%s ]\n",buffer);
					printf("then\n");
					
					//print out everything after "then" and before "fi"
					while(fgets(line, sizeof(line), file) != NULL && strstr(line, "fi") == NULL)
					{
						printf(line);
					}
					
					
					//add the "fi"
					printf("fi\n\n");
					
            }
			
			//handle the while/loop part
            if(strstr(line, "repeat") != NULL)
            {
				//storage
				char* whilec;
				
				//split it up by a space
				whilec = strtok(line, " ");
				whilec = strtok(NULL, line);
				
				//add something to increment
				printf("repeatIndex=0\n\n");
				
				//print the valid while loop format
				printf("while [ $repeatIndex -lt %s]\n",whilec);
				printf("do");
				
				fgets(line, sizeof(line), file);
				
				//print everything between the  { and }
				while(fgets(line, sizeof(line), file) != NULL && strstr(line, "}") == NULL)
				{
					if(strstr(line, "{") == NULL)
					{
						printf(line);
					}
				}
				
				//increment so the loop doesn't run indefinitely and close the loop
				printf("\trepeatIndex=$[repeatIndex + 1]\n");
				printf("done\n");				
				
            }
        }
        //close our stream
        fclose(file);
    }

	//we can't open the file
    else
    {
        printf("Error opening file: %s\n", path);
    }
}

//mywhoami function
void dowhoami()
{
	
	//passwd struct pointer
	struct passwd *user;
	
	//get the effective id using the userid
	user = getpwuid(geteuid());
	
	//print it out
	//user is a pointer to a struct
	//so the name is actually in user->pw_name
	printf("You're logged in as \"%s\".\n", user->pw_name);
}
