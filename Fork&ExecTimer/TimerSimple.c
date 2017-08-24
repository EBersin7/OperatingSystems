/*The TimerSimple program runs in conjunction with the ApplicationSimple program.
 This program first reports the time immediately before calling the function fork.
 This time is used to compare to the startup time reported by the Application. The
 fork function creates a child process which is a copy of Timer and executes its
 own code. The code the child process executes is the execution of the Application
 process. After launching the App program, we see when it actually begins to run.
*/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){
//time before the fork
	struct timeval tv1;
	double time1;
	gettimeofday(&tv1, NULL);
	time1 = tv1.tv_usec;
	printf("Timer: I am about to fork!\nThe time at the start in micro seconds is %f\n", time1);

//fork to create a copy program which executes the Application
	if(fork() == 0){
	//in the child
		execvp("./ApplicationSimple", NULL);
	}else{
		return 0;
	}
}

