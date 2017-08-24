/*The ApplicationModified is used in conjunction with the TimerModified program.
 This program is launched by the Timer program twice in order to obtain different
 times than the simple version which only launches one process. The first thing
 that this program will do is report the time in microseconds. This value is used
 to compare against the value reported in Timer before the fork. After reporting
 the start time, the random funtion is seeded with the time. Variables are then
 declared and a loop is created. The loop repeats for several minutes, each time
 creating 10 records of 120 random characters each and storing them in a file passed as an
 argument. A randomly selected record is then checked for accuracy in the file
 and in the stored written records. When the loop reaches the end, it rewinds to
 the start of the file and repeats the process. After the loop has finished, the
 file is closed and deleted. Measurements of time are again taken, but here they
 are merely for an unrelated measurement of the total run time.
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
	printf("Application %s: I have started!\nThe number of micro seconds at the start is %f\n", argv[2], time2);

//seed the rand function with the time
	srand(time2);

//large integer is used to ensure that this process will take time running while the other process begins to run
	int N = 10000000;
	int i = 0;

//creates the file to be used for storing records and checks for any errors
	FILE *fp;
	if((fp = fopen(argv[1], "w+")) == NULL){
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
	remove(argv[1]);

//fetch the time when the loop finishes and find the run time
	gettimeofday(&tv2, NULL);
	double time1 = tv2.tv_usec;
	printf("Application %s: I have finished!\nThe number of micro seconds at the end is %f\n", argv[2], time1);
	printf("The total elapsed time in micro seconds is %f\n", (time1 - time2));

	return 0;

}
