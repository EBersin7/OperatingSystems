/*
 * Edward Bersin : tuf18309
 * SHELL PROJECT
 * This program is designed to implement a simple UNIX shell. The main makes use
 * of several important functions to gather and process user commands in a cycle.
 * These functions are: read_line, parse_line, execute, and launches. As the names
 * suggest, they read a line, split the line into tokens, and then determine which
 * launch function to execute. All of these functions are executed in a loop until
 * the user quits the shell. Reading of lines is done through a string, buffer,
 * and getline. The parsing makes use of an array of strings and the strtok function
 * to gather all command line input as separate arguments. The execute function
 * checks for special symbols within the arguments in order to decide which function
 * must launch the commands. The three launch functions handle specifically,
 * background processes, redirection, and pipes. All three versions make use
 * of the fork and exec system calls to create child processes that become the
 * given program. However, they each contain small added pieces that enable their
 * self-titled launch type. The redirection launch is used by default if no special symbols
 * are found within the arguments. The shell is also capable of performing any of the
 * several built-in commands, namely: cd, clr, dir, echo, _environ, help, _pause, and quit.
 * The shell can take its commands from a file at start time. When the compiled
 * shell is run with an argument, the argument is assumed to be a file which contains
 * lines of commands. The lines are gathered, parsed, and executed in a similar manner to
 * the user prompts. When the EOF is reached, the shell exits. For more information
 * on the capabilities and the limitations of this shell please see the help manual.
 * The help command will display the proper usage of internal functions by opening the readme.
 * The readme will also give information on the usage and limitations of special symbols.
 */

//necessary libraries for the program
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//for use with the strtok function, delimiters are space characters
#define TOK_BUFSIZE 64
#define DELIMITERS " \t\n\r\a\v\f"

//used to print out the environment variables
extern char **environ;

//declaration of read, parse, execute, and launch functions
char *read_line(void);
char **parse_line(char *line);
int execute(char **args);
int launch_no_wait(char **args);
int redirection(char **args);
int pipes(char **args);

//declaration of internal functions
int cd(char **args);
int clr(char **args);
int dir(char **args);
int _environ(char **args);
int echo(char **args);
int help(char **args);
int _pause(char **args);
int quit(char **args);

//array containing a list of the internal functions
char *internals_str[] = {
    "cd", "clr", "dir", "_environ", "echo", "help", "_pause", "quit"
};

//array containing pointers to each of the internal functions
int (*internals_func[]) (char **) = {
    &cd, &clr, &dir, &_environ, &echo, &help, &_pause, &quit
};

//returns the total number of internal functions
int num_internals() {
    return sizeof (internals_str) / sizeof (char *);
}

//main entry point in the code; creates several variables, prints welcome message, begins loop
int main(int argc, char **argv) {

    //declaration of variables to be used for storing the command line
    char *line;
    char **args;
    int status = 1;

    //prints welcome message
    printf("Welcome to Edward's Shell!\nPlease see the help page for information on supported commands and operations.\n");

    //begin run of the loop
    do {
        //gets and prints the current directory
        char buf[PATH_MAX + 1];
        char *cwd = getcwd(buf, PATH_MAX + 1);
        printf("%s/myshell>\n", cwd);

        //determines whether a command line argument was passed or not
        if (argc < 2) {
            //no command file
            line = read_line();
            args = parse_line(line);
            status = execute(args);
        } else {
            //given a command file, must open and read lines from the file
            FILE *fp;
            line = NULL;
            size_t len = 0;

            //error check the opening of the file
            fp = fopen(argv[1], "r");
            if (fp == NULL) {
                exit(EXIT_FAILURE);
            }

            //loop through the file and execute each line
            while (((getline(&line, &len, fp)) != -1) && (status != 0)) {
                printf("%s", line);
                args = parse_line(line);
                status = execute(args);
            }
            status = 0;
            fclose(fp);
        }
        free(line);
        free(args);
    } while (status);

    return EXIT_SUCCESS;
}

/*
 * INTERNAL FUNCTIONS
 * INCLUDES THE FOLLOWING:
 * CD, CLR, DIR, _ENVIRON, ECHO, HELP, _PAUSE, QUIT
 */

//internal function cd, changes directory to args[1] or if no arguments given it prints the current directory
int cd(char **args) {
    char cwd[1024];
    //checks for an argument
    if (args[1] == NULL) {
        //prints the current directory if no argument
        if (getcwd(cwd, sizeof (cwd)) != NULL) {
            printf("No directory given as an argument.\nThe current working directory is: %s\n", cwd);
        } else {
            perror("getcwd() error.");
        }
    } else {
        //changes the directory to the argument
        if (chdir(args[1]) != 0) {
            perror("Error changing directory");
        }
        //gets the new directory and sets the environment variable PWD to it
        if (getcwd(cwd, sizeof (cwd)) != NULL) {
            setenv("PWD", cwd, 1);
        }
    }
    return 1;
}

//internal function clr, clears the terminal screen by using an ANSI escape sequence
int clr(char **args) {
    //this specific escape sequence moves all text to above the top line of the terminal
    printf("\e[1;1H\e[2J");
    return 1;
}

//internal function dir, prints out the contents of the given directory similar to the ls command
int dir(char **args) {
    //struct used to access the directory
    DIR *dp;
    struct dirent *ep;

    if (args[1] == NULL) {
        //no argument given, prints out the current working directory
        char buf[PATH_MAX + 1];
        char *cwd = getcwd(buf, PATH_MAX + 1);
        printf("No directory given as an argument.\nThe current working directory is: %s\n", cwd);
        dp = opendir(cwd);
    } else {
        //open the directory argument
        dp = opendir(args[1]);
    }
    if (dp != NULL) {
        //cycles through the elements in the directory and prints them out
        while (ep = readdir(dp)) {
            puts(ep->d_name);
        }
        (void) closedir(dp);
    } else {
        perror("Couldn't open the directory");
    }
    return 1;
}

//internal function _environ, prints out all of the environment variables
int _environ(char **args) {
    int i;
    //uses the extern char to gain access to the environment variables
    char *s = *environ;

    //cycles through and prints out each environment variable one per line
    for (i = 1; s; i++) {
        printf("%s\n", s);
        s = *(environ + i);
    }
    return 1;
}

//internal function echo, prints out the string of arguments given after the echo command
int echo(char **args) {
    int i = 1;
    //cycles through all arguments after echo and prints them out
    while (args[i] != NULL) {
        printf("%s ", *(args + i));
        i++;
    }
    printf("\n");
    return 1;
}

//internal function help, brings up the instructions for using my shell
int help(char **args) {

    //declaration of variables used for opening and displaying the readme
    //the value of totalLines is specific to the readme included
    FILE *fp = NULL;
    int lineCount = 0;
    int totalLines = 100;
    char ch[256];

    //check the current directory to find and open readme
    fp = fopen("./readme", "r");
    if (fp == NULL) {
        fputs("Unable to open readme file.", stdout);
        exit(EXIT_FAILURE);
    }

    //brings up the first 10 lines of the readme, user can request 10 more lines or exit
    while (fgets(ch, 256, fp) != NULL) {
        lineCount++;
        fputs(ch, stdout);

        //exit if final line is reached
        if (lineCount == totalLines) {
            return 1;
        }

        //stop giving help lines unless requested by user
        if (lineCount == 10) {
            char c;
            printf("Press 'enter' to read more or press 'q' to quit.\n");
            c = getchar();
            if (c == 'q') {
                return 1;
            } else {
                lineCount = 0;
            }
        }
    }
    return 1;
}

//internal function _pause, uses a loop and getchar to wait for user to input "enter" before returning control
int _pause(char **args) {
    char c;
    //display pause message
    printf("PAUSED\n");
    //get next character and do nothing until the user presses 'enter'
    while ((c = getchar()) != '\n') {
    }
    return 1;
}

//internal function quit, returns 0 to status variable which breaks the shell loop
int quit(char **args) {
    //0 is false, therefore this causes while condition to stop
    return 0;
}

/*
 * IMPORTANT LOOP FUNCTIONS
 * INCLUDES THE FOLLOWING:
 * READLINE, PARSELINE, EXECUTE, LAUNCH
 */

//reads and returns the line of text entered by the user from stdin
char *read_line(void) {
    //declare the string to store the line
    char *line = NULL;
    ssize_t bufsize = 0; 
    //getline allocates a buffer
    getline(&line, &bufsize, stdin);
    return line;
}

//parses the line and returns the array of tokens which will be used as arguments
char **parse_line(char *line) {
    //declaration of variables used for storing the separate pieces of the command line
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof (char*));
    char *token, **tokens_backup;

    //error check allocation
    if (!tokens) {
        fprintf(stderr, "Error allocating memory.\n");
        exit(EXIT_FAILURE);
    }

    //get the first token from the line
    token = strtok(line, DELIMITERS);
    //cycle through collecting the rest of the tokens
    while (token != NULL) {
        tokens[position] = token;
        position++;

        //reallocate memory in case there are more tokens than space
        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof (char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "Error allocating memory.\n");
                exit(EXIT_FAILURE);
            }
        }

        //temporary store the next token
        token = strtok(NULL, DELIMITERS);
    }
    //set the final token to null to indicate the termination
    tokens[position] = NULL;
    return tokens;
}

//executes either the internal function or launches a program based on the given arguments
int execute(char **args) {
    int i;

    //no command given
    if (args[0] == NULL) {
        return 1;
    }

    //special indicators for use of non-default launch function
    int wait = 0, pipe = 0;

    //search for the special symbols and indicate if found
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp("&", args[i]) == 0) {
            wait = 1;
        } else if (strcmp("|", args[i]) == 0) {
            pipe = 1;
        }
    }

    //look to use an internal function if there were no special symbols
    if ((!wait) && (!pipe)) {
        for (i = 0; i < num_internals(); i++) {
            if (strcmp(args[0], internals_str[i]) == 0) {
                return (*internals_func[i])(args);
            }
        }
    }

    //use background launching
    if (wait) {
        return launch_no_wait(args);
    } else if (pipe) {
        //use pipe launching
        return pipes(args);
    } else {
        //use default launching which also handles redirection
        return redirection(args);
    }
}

//launches a program using fork and exec and then immediately returns to the shell prompt
int launch_no_wait(char **args) {
    //used to identify child processes
    pid_t pid;

    //create a child process
    pid = fork();
    if (pid == 0) {
        //in the child process, execute the given command
        if ((execvp(args[0], args)) == -1) {
            perror("Program invocation has failed");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        //error occurred with fork
        perror("Forking has failed");
    } else {
        //in the parent process, return to the shell prompt instead of waiting for the child to finish
        return 1;
    }
}

//launches a program using fork and exec, waits for the child process to finish, also handles I/O redirects
int redirection(char **args) {
    //variables for determining child processes
    pid_t pid;
    int status = 0;

    //variables to indicate whether there is redirection and where
    int i, j = 0, k = 0, l = 0, input = 0, output = 0, append = 0, fargs = 1;

    //cycles through the arguments to find any redirection symbols
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp("<", args[i]) == 0) {
            input = 1;
            j = i;
        } else if (strcmp(">", args[i]) == 0) {
            output = 1;
            k = i;
        } else if (strcmp(">>", args[i]) == 0) {
            append = 1;
            l = i;
        }
    }

    //creates a child process
    pid = fork();
    if (pid == 0) {
        //in the child process
        if (input) {
            //change file descriptors from stdin to args[j+1] if redirecting input
            int fd0;
            if ((fd0 = open(args[j + 1], O_RDONLY)) < 0) {
                printf("error opening %s for reading\n", args[j + 1]);
                fargs = 0;
            }
            dup2(fd0, STDIN_FILENO);
            close(fd0);
        }
        if (output) {
            //change file descriptors from stdout to args[k+1] if redirecting output
            int fd1;
            if ((fd1 = open(args[k + 1], O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
                printf("error opening %s for writing\n", args[k + 1]);
                fargs = 0;
            }
            dup2(fd1, STDOUT_FILENO);
            close(fd1);
        }
        if (append) {
            //change file descriptors from stdout to args[l+1] if redirecting output
            int fd1;
            if ((fd1 = open(args[l + 1], O_RDWR | O_CREAT | O_APPEND, 0666)) < 0) {
                printf("error opening %s for writing\n", args[l + 1]);
                fargs = 0;
            }
            dup2(fd1, STDOUT_FILENO);
            close(fd1);
        }
        //check that there were actually arguments passed from redirects
        if (fargs) {
	 if(input){
		char *args2[2];
		strncpy(args[2], args2[0], strlen(args[2]));
		args2[1] = '\0';
            if ((execvp(args[0], args2)) == -1) {
               perror("Program invocation has failed");
            }
	  }else{
		if((execvp(args[0], args)) == -1){
		perror("Program invocation has failed");
	    }
	  }
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        //error occurred with fork
        perror("Forking has failed");
    } else {
        //in the parent process
        do {
            //wait for the child process to finish before returning control of the shell to the user
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

//launches a program using fork and exec, handles pipes for two commands with no arguments
int pipes(char **args) {
    //variables for file descriptors and child processes
    int fd[2];
    pid_t pid;

    //find the position of the pipe
    int i, j = 0;
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp("|", args[i]) == 0) {
            j = i;
        }
    }

    //create the pipe
    if (pipe(fd) == -1) {
        printf("pipe error\n");
        return EXIT_FAILURE;
    }

    //create a child process
    pid = fork();
    if (pid == 0) {
        //in the child process
        //replace standard output with the output half of the pipe
        dup2(fd[1], STDOUT_FILENO);
        //close the unused input half of the pipe
        close(fd[0]);
        close(fd[1]);
        //execute the first program
        if ((execlp(args[j - 1], args[j - 1], NULL)) == -1) {
            perror("Program invocation has failed");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        //error occurred with fork
        perror("Forking has failed");
    } else {
        //in the parent process
        //create another process
        pid = fork();
        if (pid == 0) {
            //in the child process
            //replace standard input with the input half of the pipe
            dup2(fd[0], STDIN_FILENO);
            //close the unused output half of the pipe
            close(fd[1]);
            close(fd[0]);
            //execute the second program
            if ((execlp(args[j + 1], args[j + 1], NULL)) == -1) {
                perror("Program invocation has failed");
            }
        } else if (pid < 0) {
            //error occurred with fork
            perror("Forking has failed");
        }
    }
    //close the file descriptors and return from the parent
    close(fd[0]);
    close(fd[1]);
    return 1;
}
