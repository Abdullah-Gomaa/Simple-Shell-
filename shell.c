/*************************************** Libraries Required *******************************************/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>
/**************************************** Pre-defined Macros ******************************************/
#define shell_builtin             0
#define executable_or_error       1
/**************************************** Functions Protoypes ****************************************/
void parseInput();
void sig_handler();
int inputType();
void execute_shell_builtin(char* pr);
void execute_command();
void getValueFromKey(char* key, char** ret_val);
/***************************************** Global Variables ******************************************/
char p[256];
int cd_flag = 0;
char* args[20];
int  flag = 0;
char variables_names[20][30];
char variables_values[20][30];
int variables_index = 0;
char* cmd = 0;
char arr[100];
char* command[100];
char* path = NULL;
/****************************************** Signal Handler ******************************************/
void sig_handler()
{
    pid_t pid;
    int status;

    // Wait for any child process to terminate
        pid = waitpid(-1, &status, WNOHANG);
    
    // Enters the loop that continues as long as there are child processes to wait for
        while(pid>0)
        {
        // Debugging print statments for status of child termination
            /* 
            if (WIFEXITED(status)) {
                printf("Child process %d exited with status %d\n", pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process %d terminated by signal %d\n", pid, WTERMSIG(status));
            }
            */

        // Create a log file if it is not created and open it
            FILE *f = fopen("log.txt", "a");
            if(f==NULL)
            {   
                perror("Error in open log file");
            }
            else
            {
                // Add the string in a new line when a child process is terminated
                fprintf(f, "Child Terminated\n");
            }
            fclose(f);
        // update wait pid status    
            pid = waitpid(-1, &status, WNOHANG);
        }
    
}
/****************************************** main function ********************************************/
/*___________________________________________________________________________________________________*/
int main(void)
{
    // Char array to store current directory
    char currdir[256];
    // Register a signal handler for SIGCHILD signal 
    signal(SIGCHLD, sig_handler);
    
    // Loop continously until exit command 
    while (1)
    {
        // Set working enviroment to current directory
            getcwd(currdir, 256);
            chdir(currdir);

        // Take input from user
            parseInput();
        
        // Check if command entered by user is exit to break the loop and end the program    
            if(!strcmp(cmd, "exit"))
                break;


        // Check if command is shell built-in or exectuable or error        
            if(inputType() == shell_builtin)
            {
                execute_shell_builtin(arr);

                // Clear arr for next input
                memset(arr, 0, sizeof(arr));
            }
            else if(inputType() == executable_or_error)
            {
                execute_command();
            }
    }
    
    return 0;
}
/*___________________________________________________________________________________________________*/


/******************************************* Parsing Input *******************************************/
void parseInput()
{
    // Prepare variables for recieving next input
        memset(command, 0, sizeof(command));
        memset(p, 0, sizeof(p));
    // Initialize variables and set delimiter    
        int i=0 , j=0;
        char buffer[512];
        char delim[] = " ";

    // Recieve input from the user and store it in buffer  
        fgets(buffer, 512, stdin);

    // Remove new line feed at the end of input string
        buffer[strcspn(buffer, "\n")] = 0;
    
    // Seperate input string to seperate words and store every word in args array
        char *ptr = strtok(buffer, delim);
        while(ptr!=NULL)
        {
            args[i] = ptr;
            ptr = strtok(NULL, delim);
            i++;
        }
        args[i] = NULL;
    
    // Store the command in cmd 
        cmd = args[0];
    
    // if the command is cd store path entered in path variable
        if(strcmp(cmd, "cd") == 0)
        {
            path = args[1];
        }

    // Store the complete command entered in command array to be used in execvp
        i=0;
        while(args[i] != NULL)
        {
            command[i] = args[i];
            i++;
        }
        command[i] = NULL; 

    // Check for '&' for background process
        if(command[1]!= NULL )
        {
            if(command[1][0] == '&')
            {
                command[1] = NULL;
                // Set flag for background process
                flag =1;
            }
        }

    // Prepare the proper argument for cd command
        if(command[1] == NULL)
        {
            // Set cd_flag if required directory is home 
            cd_flag = 1;
        }
        else if(command[1][0] == '~')
        {
            // Set cd_flag if required directory is home
            cd_flag = 1;
        }
        else
        {
            cd_flag = 0;
            // Prepare the path in suitable form for passing to chdir()
                path = command[1];
                for(int i=0; i<strlen(path); i++)
                {
                    p[i] = path[i];
                } 
        }
    
    // Concatenate all arguments after the first word and store them in arr[] with spaces
        i=1; j=0;
        int k=0;
        while(args[i]!=NULL)
        {
            for(j=0; j<strlen(args[i]); j++)
            {
                arr[k] = args[i][j];
                k++;
            }
            arr[k++] = ' ';
            i++;
        }

    // Remove all quotes from arr
        // Iterate over each character in the string
        for (i = 0, j = 0; arr[i] != '\0'; i++) {
            // If the current character is not a quote, copy it to the output string
            if (arr[i] != '"') {
                arr[j] = arr[i];
                j++;
            }
        }
        // Terminate the output string with a null character
        arr[j] = '\0'; 
}
/*___________________________________________________________________________________________________*/

/********************************************* Input Type *********************************************/
int inputType()
{
    // Check for shell built-in or executable or error
        if( !strcmp(cmd,"cd") || !strcmp(cmd,"echo") || !strcmp(cmd,"export"))
            return shell_builtin;
        else
            return executable_or_error;
}
/*___________________________________________________________________________________________________*/

/******************************************* Shell Built-in *******************************************/
void execute_shell_builtin(char* pr)
{
    // Initialize variables
        char dir[256];
        int i =0, j=0, c=0;
        int index = 0;
        char temp[30];
        char* ptr = NULL;
        char * key = NULL;
        char * value = NULL;
    /********************** cd ********************* */
   if(strcmp(cmd, "cd") == 0)
   {
        // Check cd flag for home directory 
            if(cd_flag == 1)
            {
                // Get user home directory path
                path = getenv("HOME");
                for(int i=0; i<strlen(path); i++)
                    p[i] = path[i];
            }

        // Change the directory to the required path    
            chdir(p);
        
        getcwd(dir, 256);
   }
   /********************** echo ********************* */
   if( strcmp(cmd, "echo") == 0)
   {    
        // Check if input string contain a variable to be exported
            char * pos = strchr(pr, '$');
            if(pos!=NULL)
            {
                // Check if echo has no string but $ variable
                if(pr[0]=='$')
                {  
                    while(pr[c]!=0)
                    {
                        pr[c] = pr[c+1];
                        c++;
                    }
                     // Get the options as string from the value stored in the variable
                    getValueFromKey(pr, &value);
                    printf("%s\n", value);
                }
                else
                {
                    // Get variable name from string
                        ptr = strtok(pr, "$");
                        char* before = ptr;
                        ptr = strtok(NULL, "$");
                        char *after = ptr;

                    // Remove space at the end of string after $   
                        if(after!=NULL)
                            after[strcspn(after, " ")] = 0;
                        
                    // Check if variable is after a string to determine key    
                        if(after == NULL)
                            key = before;
                        else
                            key = after;   
                    
                    // Get index for the value corresponding to the required key
                        for(int i=0; i<30; i++)
                        {
                            if(strcmp(variables_names[i],key)==0)
                            {
                                index = i;
                                break;
                            }
                        } 
                    // Get the value of the key using the index
                        value = variables_values[index];

                    // Prepare suitable output format
                        if(after != NULL)
                            strcat(before, value);
                        else 
                            before = value;

                    // Print output string after replacing variable with value            
                        printf("%s\n", before);
                }
            }   
            // Condition: String does not contain $        
            else
            {
                // Print string entered by user
                    printf("%s\n", pr);
            }
   }
   /********************** export ********************* */
   else if ( strcmp(cmd, "export") == 0)
   {
        // Get the variable name
            while(pr[i]!= '=')
            {
                temp[i] = pr[i];
                i++;
            }
        // Store variable name in the array variable_names    
            strcpy(variables_names[variables_index],temp);
        // Clear temp array for getting value of the variable    
            memset(temp, 0, sizeof(temp));
        // Increament the iterator for string entered     
            i++;

        // Get value    
            while (pr[i]!='\0')
            {
                temp[j++] = pr[i++];
            }

        // Store variable value in the array variable_values
            strcpy(variables_values[variables_index],temp);
        
        // Increament index for storing multiple variables 
            variables_index++;

        // Clear temp array for future inputs
            memset(temp, 0, sizeof(temp));
   }
    
}
/*___________________________________________________________________________________________________*/

/****************************************** Execute Commands ******************************************/
void execute_command()
{
    // Initialize variables
        char* options = NULL;
        int wait_status =0;
        int w ;
    
    // Create child process
         __pid_t pid = fork();
    if(pid==0)
    {
    // Child process created successfully
        // Check if command is executable 
        if(inputType() == executable_or_error)
        {   
            // Check if command option starts with $ 
                if((command[1] != NULL) && (command[1][0] == '$'))
                {
                    // Remove $ sign to get variable name
                    strncpy(command[1], command[1]+1, strlen(command[1])-1);
                    command[1][strlen(command[1])-1] = '\0';

                    // Get the options as string from the value stored in the variable
                    getValueFromKey(command[1], &options);

                    // Prepare command array by seperating options from the string
                        char * tok = strtok(options, " ");
                        int i =1;
                        while(tok != NULL)
                        {
                            command[i] = tok;
                            tok = strtok(NULL, " ");
                            i++;
                        }
                }
            // Execute the command
            execvp(command[0], command);
            
            // exevp failure errors
            perror("Error in execvp");
            exit(EXIT_FAILURE);
        }
        
    }
    else if(pid == -1)
    {
        // Child process creation failed
        perror("Error in Fork");
    }
    else
    {
    // Parent process
        if(flag == 0)
        {
            sleep(1);
            w = waitpid(-1, &wait_status, 0);
            // return;
        }
        else
        {
            // wait for child process to complete
            w = waitpid(-1, &wait_status, WNOHANG);
            if(w == -1)
            {
                // wait failure error
                perror("Error in waitpid");
                exit(EXIT_FAILURE);
            }  
        }
        
    }
}
/*___________________________________________________________________________________________________*/

/*************************************** Helping Function ********************************************/
void getValueFromKey(char* key, char** ret_val)
{
    // Initialize variables
        int index = 0;
        char *value = NULL;

    // Get index of value from variable name    
    for(int i=0; i<30; i++)
    {
        if(strcmp(variables_names[i],key)==0)
        {
            index = i;
            break;
        }
    } 

    // Get value
    value = variables_values[index];
    // store the value in the varibale passed by reference
    *ret_val = value;
}
/********************************************** END ***************************************************/