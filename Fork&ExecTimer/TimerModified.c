/*The TimerModified program works in conjunction with the ApplicationModified
 program. Similarly to the TimerSimple program, this program first reports the
 time in microseconds before the first fork. The child process created by this
 fork then goes on to create arguments necessary for the ApplicationModified to
 use before launching it with said arguments. This program however, continues
 by reporting the time again in the parent process before another fork occurs.
 This second child process does the same exact thing as the first child process,
 albeit with a few changes to the argument names. Each Application will report the
 time it starts, thus allowing for comparison between start times of a second process.
*/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){
//time before the fork
	struct timeval tv1;
	double time1;
	gettimeofday(&tv1, NULL);
	time1 = tv1.tv_usec;
	printf("Timer: I am about to fork!\nThe current time in micro seconds is %f\n", time1);

//fork once, creating a child process that executes the Application and gives it a unique file to use
	if(fork() == 0){
	//in the child, creating arguments for first process
		char *args[4];
		args[0] = "ApplicationModified"; //executable name
		args[1] = "RecordsOne"; //File created by executable
		args[2] = "IndexOne"; //Index to keep track of processes
		args[3] = NULL; //args must end with a NULL element
		execvp("./ApplicationModified", args);
	}else{
	//in the parent process, measure time before next fork
		gettimeofday(&tv1, NULL);
		time1 = tv1.tv_usec;
		printf("Timer: I am about to fork again!\nThe current time in micro seconds is %f\n", time1);
		if(fork() == 0){
		//in the new child
			char *args[4];
			args[0] = "ApplicationModified";
			args[1] = "RecordsTwo";
			args[2] = "IndexTwo";
			args[3] = NULL;
			execvp("./ApplicationModified", args);
		}else{
		//in the parent process
			int status;
			wait(&status);
		}

	return 0;
	}
}

