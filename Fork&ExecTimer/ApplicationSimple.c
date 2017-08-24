/*The ApplicationSimple program works in conjunction with the TimerSimple program.
 This version was my first use for testing and making sure each part of the process
 actually worked correctly. The program here creates and uses a file, records.txt,
 to perform the actions in the loop instead of receiving command line arguments.
 The loop lasts for a shorter period of time so that I could more quickly gather
 results and see the outcomes. In all other  aspects, this program is identical
 to the ApplicationModified.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char **argv){

//struct timeval is used to get the time that this process first starts in micro seconds
	struct timeval tv2;
	double time2;
	gettimeofday(&tv2, NULL);
	time2 = tv2.tv_usec;
	printf("Application: I have started!\nThe number of micro seconds at the start is %f\n", time2);

//seed the rand function with the time
	srand(time2);

//smaller integer is used because we are only trying to measure startup time of one process
	int N = 100000;
	int i = 0;

//creates the file to be used for storing records and checks for any errors
	FILE *fp;
	if((fp = fopen("records.txt", "w+")) == NULL){
		printf("Error creating file!");
	}

//the array used for the record comparison and ints for the loops
	char records[10][120];
	int j, k;

//start of the while loop which will create, save, and compare records
	while(i < N){
//for loop used for the 10 records
		for(j = 0; j < 10; j++){
//for loop used for the 120 characters per record
			for(k = 0; k < 120; k++){
//generates a random character using ascii values for digits, letters, and some symbols
				int asciiVal = rand()%75 + 48;
        			char asciiChar = asciiVal;
				records[j][k] = asciiChar;
				fwrite(&asciiChar, sizeof(char), 1, fp);
			}
			fwrite("\n", sizeof(char), 1, fp);
		}
//select a random record, read it from the file, and compare to the written
		int select = rand()%10;
		fseek(fp, select*(120 + 1), SEEK_SET);
		char buffer[120];
		fread(buffer, sizeof(char), 120, fp);
		int boolean = 0;
		for(j = 0; (boolean!=1) && (j < 120); j++){
			if(records[select][j]!=buffer[j]){
				printf("There was a mismatch at %d!\n",j);
				boolean = 1;
			}
		}
//finished with this loop, rewind the file and increment
		rewind(fp);
		i++;
	}

//close and delete the file
	fclose(fp);
	remove("records.txt");

//fetch the time when the loop finishes and find the run time of Application
	gettimeofday(&tv2, NULL);
	double time1 = tv2.tv_usec;
	printf("Application: I have finished!\nThe number of micro seconds at the end is %f\n", time1);
	printf("The total elapsed time in micro seconds is %f\n", (time1 - time2));

	return 0;

}
