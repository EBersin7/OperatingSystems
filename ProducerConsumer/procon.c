/*
 * Edward Bersin : tuf18309@temple.edu
 * PRODUCER_CONSUMER
 * Solution to the producer-consumer problem using a mutex and condition variables.
 * This solution was developed for the Linux development environment.
 * A struct is used to keep track of the important varibles in the program.
 * The main initializes the struct, creates the threads, and waits for them to finish run.
 * The solution contains sleep timers to vary the speeds of threads based on id.
 * Many different interleavings occur, but the threads remain safe as evidenced by the
 * various print statements used to indicate their operation. Timestamps print as well
 * as the items being removed by which thread and from which slot.
 */

//included libraries for use with I/O and thread functions
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

//constants for the buffer size and the number of consumers/producers
#define BUFSIZE 25
#define NC 5
#define NP 5

//function declarations for the producers/consumers and initialization
void *producer(void *arg);
void *consumer(void *arg);
void sbuf_init();

//struct which contains the buffer, item count, start/end point, mutex, and condition variables

typedef struct {
    int buffer[BUFSIZE];
    int count;
    int start;
    int end;
    pthread_mutex_t mutex;
    pthread_cond_t empty;
    pthread_cond_t full;

} sbuf_t;

//struct declaration
sbuf_t sp;

int main(int argc, char **argv) {

    //initialization of the struct variables
    sbuf_init();

    //declaration of the producer/consumer threads
    pthread_t producers[NP];
    pthread_t consumers[NC];

    //counting and id variables
    int i;
    int *id_p;
    int *id_c;

    //create the producers
    for (i = 0; i < NP; i++) {
        if ((id_p = (int*) malloc(sizeof (int))) == NULL)
            exit(EXIT_FAILURE);
        *id_p = i;
        pthread_create(&producers[i], NULL, producer, id_p);
        printf("Producer thread %d created.\n", i);
    }

    //create the consumers
    for (i = 0; i < NC; i++) {
        if ((id_c = (int*) malloc(sizeof (int))) == NULL)
            exit(EXIT_FAILURE);
        *id_c = i;
        pthread_create(&consumers[i], NULL, consumer, id_c);
        printf("Consumer thread %d created.\n", i);
    }

    //join all the threads so that main does not exit before they run
    for (i = 0; i < 5; i++) {
        pthread_join(producers[i], NULL);
    }
    for (i = 0; i < 5; i++) {
        pthread_join(consumers[i], NULL);
    }

    return 0;
}

//initializes the count, start/end, mutex, and condition variables

void sbuf_init() {
    sp.count = sp.start = sp.end = 0;
    pthread_mutex_init(&sp.mutex, NULL);
    pthread_cond_init(&sp.empty, NULL);
    pthread_cond_init(&sp.full, NULL);

}

//producers create an item at random and try to add it to the buffer
void *producer(void *arg) {

    //get the id value passed to the thread and declare item to be produced
    int id = *(int*) arg;
    int item;

    //seed the random function
    srand((unsigned) time(NULL));

    //vary the speed of starting threads by sleeping for number of seconds = id
    printf("Producer thread %d starts in %d seconds.\n", id, id);
    sleep(id);

    while (1) {
        //entering the critical section
        pthread_mutex_lock(&sp.mutex);

        //get a timestamp for first acquiring the mutex
        time_t ltime;
        struct tm result;
        char stime[32];
        ltime = time(NULL);
        localtime_r(&ltime, &result);
        asctime_r(&result, stime);

        printf("Producer thread %d acquired the mutex at %s", id, stime);

        //check the condition variable
        while (sp.count == BUFSIZE) {
            printf("All slots are currently filled.\n");
            printf("The producer is waiting for an open slot.\n");
            pthread_cond_wait(&sp.empty, &sp.mutex);
        }

        //obtain a random int value
        item = rand() % 1000;
        //place the item into the last position in the buffer
        sp.buffer[sp.end] = item;
        printf("Producer thread %d put %d in slot #%d.\n", id, item, sp.end);
        //adjust the buffer
        sp.end = (sp.end + 1) % BUFSIZE;
        sp.count++;

        //timestamp before letting go of the lock
        ltime = time(NULL);
        localtime_r(&ltime, &result);
        asctime_r(&result, stime);
        printf("Producer thread %d released the mutex at %s", id, stime);

        //leaving the critical section
        pthread_mutex_unlock(&sp.mutex);

        //wake up a consumer
        pthread_cond_signal(&sp.full);

        //sleep for a bit to give others a chance
        sleep(1);
    }
}

//consumers remove an item from opposite end that the producers have been inserting
void *consumer(void *arg) {

    //get the id value passed to the thread and declare item to be produced
    int id = *(int*) arg;
    int item;

    //vary the speed of starting threads
    printf("Consumer thread %d starts in %d seconds.\n", id, id);
    sleep(0);

    while (1) {
        //entering the critical section
        pthread_mutex_lock(&sp.mutex);

        //get a timestamp for first acquiring the mutex
        time_t ltime;
        struct tm result;
        char stime[32];
        ltime = time(NULL);
        localtime_r(&ltime, &result);
        asctime_r(&result, stime);

        printf("Consumer thread %d acquired the mutex at %s", id, stime);

        //check the condition variable
        while (sp.count == 0) {
            printf("All slots are currently empty.\n");
            printf("The consumer is waiting for an item.\n");
            pthread_cond_wait(&sp.full, &sp.mutex);
        }

        //remove the item from the buffer
        item = sp.buffer[sp.start];
        printf("Consumer thread %d removed %d in slot #%d.\n", id, item, sp.start);
        sp.start = (sp.start + 1) % BUFSIZE;
        sp.count--;

        //timestamp before letting go of lock
        ltime = time(NULL);
        localtime_r(&ltime, &result);
        asctime_r(&result, stime);
        printf("Consumer thread %d released the mutex at %s", id, stime);

        //leaving the critical section
        pthread_mutex_unlock(&sp.mutex);

        //wake up a producer
        pthread_cond_signal(&sp.empty);

        //sleep for a bit to give others a chance
        sleep(1);
    }
}
