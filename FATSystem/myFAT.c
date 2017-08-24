/*
 * Edward Bersin | tuf18309@temple.edu
 * Last modified April 27th, 2016
 * The following is my attempt at creating a file system in C.
 * I have tried to recreate a simple FAT file system using the mmap function.
 * 
 * At this point, I have not finished development because of an error I encountered.
 * After successfully compiling and running the basic setup of the virtual disk, I
 * began to write and test the functions. However, each and every function produced
 * a segmentation fault. When I tried to test them just in main and not as function calls
 * I continued to receive this error. I attempted to adjust the type declarations and
 * the bounds of the array to no avail. All my attempts at fixing this issue failed
 * and I was left with an array of blocks that I could not work with.
 * 
 * In my project design doc, I described how I would attempt to recreate the FAT
 * file system by using two different structs and the mmap function. The mmap call
 * would allow the struct array/virtual disk to be manipulated as though it was
 * simply an array. The virtual disk is supposed to be a contiguous block of memory
 * so this seemed easy. However, I ended up changing details of the design as I began
 * to actually work on the code. In the end, I settled on what is basically a linked list
 * where each node is a data block and can hold key information on itself and where to find
 * the next block of each file. The manipulation of these blocks would be straight-forward
 * as each struct was able to house pieces of data for a file. The result was not what
 * I desired though and it still needs adjustments.
 */

//libraries to include for standard operations, I/O functions, and other system calls
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

//constants for use in the virtual disk creation
#define BLOCK_SIZE 512
#define NUM_BLOCKS 4096
#define FILESIZE (NUM_BLOCKS * BLOCK_SIZE)

//struct definitions
struct FAT_ENTRY {
    int index;
    struct BLOCK_ENTRY *head;
};

typedef struct FAT_ENTRY table;

struct BLOCK_ENTRY {
    int num;
    int usage;
    char *data;
    char *name;
    struct BLOCK_ENTRY *next;
};

typedef struct BLOCK_ENTRY block;

//function definitions
int myCreate();
int myOpen();
void myDelete();
void myRead();
void myWrite();
int *myClose();

int main(int argc, char *argv[]) {
    
    //counting variable, file descriptor, error check variable, and array for map
    int i;
    int fd;
    int result;
    block *map;

    //open the file given as an argument for reading and writing
    //creates the file if it does not exist or truncates it if it does exist
    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
    if (fd == -1) {
        printf("Error opening file for writing.");
        exit(EXIT_FAILURE);
    }

    //adjust the size of the file to the size of the array of structs
    if ((result = lseek(fd, FILESIZE - 1, SEEK_SET)) == -1) {
        close(fd);
        printf("Error using lseek to adjust the file size.");
        exit(EXIT_FAILURE);
    }

    //must add something to the end of the file to ensure that it is made to the
    //proper size and stays that way
    result = write(fd, "", 1);
    if (result != 1) {
        close(fd);
        printf("Error writing to the file.");
        exit(EXIT_FAILURE);
    }

    //the file is now mapped to the previously declared struct pointer
    map = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        printf("Error mmapping the file.");
        exit(EXIT_FAILURE);
    }

    //cycle through the array and initialize all of the structs
    //this setup creates a linked list structure between the blocks as well
    block *head;
    head = NULL;
    for (i = 0; i <= NUM_BLOCKS; i++) {
        map[i].num = i;
        map[i].usage = 0;
        map[i].data = "\0";
        map[i].name = "\0";
        map[i].next = head;
        head = &map[i];
    }

    //here I will test each of the IOCS functions in a sequential order
    //this causes a segmentation fault: int index = myCreate(map, "TEST FILE");
    //what is in myCreate as been copied here to test what may be the problem
    //a segmentation fault still occurs here and I could not figure out why
    /*int index = -1;
    char *name = "TEST FILE";
    printf("Searching for an empty block.");
    for (i = 0; i <=NUM_BLOCKS; i++) {
        if(map[i].usage == 0){
            printf("An empty block has been found at index %d.", i);
            map[i].usage = 1;
            strcpy(map[i].name, name);
            index = i;
            break;
        }
    }
    printf("The index of the block to be used is %d.", index);
    
    //what is in myOpen has been copied here
    //it produces the same error
    printf("Searching for the specified file name.");
    for (i = 0; i <=NUM_BLOCKS; i++) {
        if((strcmp(map[i].name, name)) == 0){
            printf("The file has been found at index %d.", i);
            index = i;
            break;
        }
    }
    */
    //finished testing of the IOCS functions

    //the mmapped memory must be freed when done being used
    if (munmap(map, FILESIZE) == -1) {
        printf("Error un-mmapping the file.");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //finish by closing the file
    close(fd);

    return 0;
}

//given the array and a file name, search for a free block and adjust its values
int myCreate(block *map, char *name) {

    int i;
    int index = -1;
    printf("Searching for an empty block.");
    for (i = 0; i <= NUM_BLOCKS; i++) {
        if (map[i].usage == 0) {
            printf("An empty block has been found at index %d.", i);
            map[i].usage = 1;
            strcpy(map[i].name, name);
            index = i;
            break;
        }
    }

    return index;
}

//given the array and a file name, search for the block and reinitialize it to 0
void myDelete(block *map, char *name) {

    int i;
    printf("Searching for the specified file name.");
    for (i = 0; i <= NUM_BLOCKS; i++) {
        if ((strcmp(map[i].name, name)) == 0) {
            printf("The file has been found at index %d.", i);
            strcpy(map[i].name, "\0");
            strcpy(map[i].data, "\0");
            map[i].usage = 0;
            break;
        }
    }
}

//given the array and a file name, search the blocks for this name and return its index
int myOpen(block *map, char *name) {

    int i;
    int index = -1;
    printf("Searching for the specified file name.");
    for (i = 0; i <= NUM_BLOCKS; i++) {
        if ((strcmp(map[i].name, name)) == 0) {
            printf("The file has been found at index %d.", i);
            index = i;
            break;
        }
    }

    return index;
}

//given the array, a file name, and some data, search for the specific block and write the data in
void myWrite(block *map, char *name, char *data) {

    int i;
    printf("Searching for the specified file name.");
    for (i = 0; i <= NUM_BLOCKS; i++) {
        if ((strcmp(map[i].name, name)) == 0) {
            printf("The file has been found at index %d.", i);
            strcpy(map[i].data, data);
            break;
        }
    }

}

//given the array and a file name, search the blocks and then read out the data
void myRead(block *map, char *name) {

    int i;
    printf("Searching for the specified file name.");
    for (i = 0; i <= NUM_BLOCKS; i++) {
        if ((strcmp(map[i].name, name)) == 0) {
            printf("The file has been found at index %d.", i);
            printf("%s", map[i].data);
            break;
        }
    }
}
