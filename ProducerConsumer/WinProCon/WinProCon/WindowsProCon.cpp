/*
* Edward Bersin : tuf18309@temple.edu
* PRODUCER_CONSUMER
* Solution to the producer-consumer problem using a mutex and semaphores.
* This solution was developed for the Windows Visual Studio development environment.
* The main initializes the struct, creates the threads, and waits for them to finish run.
* The solution contains sleep timers to vary the speeds of threads based on id.
* Many different interleavings occur, but the threads remain safe as evidenced by the
* various print statements used to indicate their operation. Timestamps print as well
* as the items being removed by which thread and from which slot.
*/

//included libraries for use with I/O and thread functions
#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <stdlib.h>
#include <ctime>
#include <mutex>
#include <thread>
#include <time.h>

//constants for the buffer size and the number of consumers/producers
#define BUFSIZE 25
#define NC 5
#define NP 5

//pointers to the semaphore objects for synchronization
HANDLE mutex;
HANDLE empty;
HANDLE full;

//declaration of functions to be used for threads
DWORD WINAPI Producer(LPVOID);
DWORD WINAPI Consumer(LPVOID);

//declaration of the buffer, endpoints, and the item count
int buffer[BUFSIZE];
int start, end;
int count;

int main(void) {

	//create the producer and consumer objects
	HANDLE producers[NP];
	HANDLE consumers[NC];

	//variables used for thread id and counting
	DWORD ThreadID;
	int i;
	start = 0; 
	end = 0;
	count = 0;

	//initialize the mutex and semaphores
	mutex = CreateMutex(NULL, 0, NULL);
	empty = CreateSemaphore(NULL, 0, 0, NULL);
	full = CreateSemaphore(NULL, 0, 0, NULL);

	//create the producer threads
	for (i = 0; i < NP; i++) {
		producers[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Producer, NULL, 0, &ThreadID);
	}

	//create the consumer threads
	for (i = 0; i < NC; i++) {
		consumers[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Consumer, NULL, 0, &ThreadID);
	}

	//wait for the threads to exit
	WaitForMultipleObjects(NP, producers, TRUE, INFINITE);
	WaitForMultipleObjects(NC, consumers, TRUE, INFINITE);

	//close the threads and the mutex handles
	for (i = 0; i < NP; i++) {
		CloseHandle(producers[i]);
	}
	for (i = 0; i < NP; i++) {
		CloseHandle(consumers[i]);
	}

	return 0;
}

DWORD WINAPI Producer(LPVOID lpParam) {

	UNREFERENCED_PARAMETER(lpParam);

	srand(time(NULL) + GetCurrentThreadId());
	int item;
	int speed = (rand() % 10) + 1;

	printf("Producer thread %d starts in %d seconds.\n", GetCurrentThreadId(), speed);
	Sleep(speed*1000);

	while (1) {

		WaitForSingleObject(empty, INFINITE);
		WaitForSingleObject(mutex, INFINITE);

		//get a timestamp for first acquiring the mutex
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

		printf("Producer thread %d acquired the mutex at %s\n", GetCurrentThreadId(), buf);

		//create the random item and insert it into the buffer
		item = rand() % 1000;
		buffer[end] = item;
		printf("Producer thread %d put %d in slot #%d.\n", GetCurrentThreadId(), item, end);
		end = (end + 1) % BUFSIZE;
		count++;
		Sleep(speed*500);

		//timestamp before letting go of the lock
		now = time(0);
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
		printf("Producer thread %d released the mutex at %s\n", GetCurrentThreadId(), buf);

		ReleaseSemaphore(full, 1, 0);
		ReleaseMutex(mutex);
	}
	return TRUE;
}

DWORD WINAPI Consumer(LPVOID lpParam) {

	UNREFERENCED_PARAMETER(lpParam);
	srand(time(NULL) + GetCurrentThreadId());
	int item;
	int speed = (rand() % 10) + 1;

	printf("Consumer thread %d starts in %d seconds.\n", GetCurrentThreadId(), speed);
	Sleep(speed*2000);

	while (1) {

		WaitForSingleObject(full, INFINITE);
		WaitForSingleObject(mutex, INFINITE);

		//get a timestamp for first acquiring the mutex
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

		printf("Consumer thread %d acquired the mutex at %s\n", GetCurrentThreadId(), buf);

		//remove the item from the buffer and adjust the buffer
		item = buffer[start];
		printf("Consumer thread %d removed %d in slot #%d.\n", GetCurrentThreadId(), item, start);
		start = (start + 1) % BUFSIZE;
		count--;
		Sleep(speed*500);

		//timestamp before letting go of the lock
		now = time(0);
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
		printf("Consumer thread %d released the mutex at %s\n", GetCurrentThreadId(), buf);

		ReleaseSemaphore(empty, 1, 0);
		ReleaseMutex(mutex);

	}
	return TRUE;
}

