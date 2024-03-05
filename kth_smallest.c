#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define READ 0
#define WRITE 1
#define NUM_CHILDREN 5
#define ARRAY_SIZE 5 // Assuming each file has 5 integers

enum Commands { REQUEST = 1, READY, PIVOT, LARGE, SMALL, STOP };

int childToParent[NUM_CHILDREN][2]; // Array to hold pipes for child to parent communication
int parentToChild[NUM_CHILDREN][2]; // Array to hold pipes for parent to child communication

void initializePipes(void) {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (pipe(childToParent[i]) == -1 || pipe(parentToChild[i]) == -1) {
            perror("pipe initialization failed");
            exit(EXIT_FAILURE);
        }
    }
}

// Helper function to write a command to a pipe
void writeCommand(int fd, int cmd) {
    ssize_t bytesWritten = write(fd, &cmd, sizeof(cmd));
    if (bytesWritten == -1) {
        perror("writeCommand failed");
        exit(EXIT_FAILURE);
    }
}

// Helper function to read a response from a pipe
void readResponse(int fd, int *response) {
    ssize_t bytesRead = read(fd, response, sizeof(*response));
    if (bytesRead == -1) {
        perror("readResponse failed");
        exit(EXIT_FAILURE);
    }
}

// Quick Sort Algorithm - Utility Function for Comparing Numbers
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

void ChildProcess(int id) {
    // Close unused pipe ends
    for (int j = 0; j < NUM_CHILDREN; j++) {
        if (j != id) {
            close(parentToChild[j][READ]);
            close(childToParent[j][WRITE]);
            close(parentToChild[j][WRITE]);
            close(childToParent[j][READ]);
        }
    }
    // Keep the read end from parent and write end to parent open for this child
    close(parentToChild[id][WRITE]);
    close(childToParent[id][READ]);


    // Read input from file
    char filename[50];
    sprintf(filename, "input_%d.txt", id + 1); // Adjust index to match file naming
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    int array[ARRAY_SIZE];
    int readCount; // Variable to store the number of items read by fscanf
    for (int i = 0; i < ARRAY_SIZE; i++) {
        readCount = fscanf(file, "%d", &array[i]);
        if (readCount < 1) {
            // If fscanf didn't successfully read an item, handle the error
            if (feof(file)) {
                // End of file reached before expected, might be fewer integers in file than expected
                fprintf(stderr, "End of file reached prematurely in file: %s. Expected %d integers.\n", filename, ARRAY_SIZE);
            } else if (ferror(file)) {
                // An error occurred during reading
                perror("Error reading from file");
            } 
            fclose(file); // Close the file before exiting
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("Child %d: array[%d] = %d\n", id, i, array[i]); //WORKS!
    }

    // Send READY signal to parent
    int message = READY;
    ssize_t bytesWritten = write(childToParent[id][WRITE], &message, sizeof(message));
    if (bytesWritten == -1) {
        perror("Child write (READY) to parent failed");
        exit(EXIT_FAILURE);
    }

    // Signal readiness
    printf("Child %d ready\n", id); //WORKS!

    int pivot;
    int arraySize = ARRAY_SIZE; // Keep track of the current size of the array
    int command;
    while (read(parentToChild[id][READ], &command, sizeof(command)) > 0) {
        
        if (command == REQUEST) {
            // Select a random value from the array and send it back to the parent
            int randomIndex = rand() % arraySize; // Ensure this doesn't select an uninitialized index
            int randomValue = array[randomIndex]; 
            write(childToParent[id][WRITE], &randomValue, sizeof(randomValue));
        } else if (command == PIVOT) {
            ssize_t bytesRead = read(parentToChild[id][READ], &pivot, sizeof(pivot));
            if (bytesRead == -1) {
                perror("Child read pivot failed");
                exit(EXIT_FAILURE);
            }

            printf("Child %d: Received PIVOT %d\n", id, pivot);

            int count = 0;
            for (int i = 0; i < arraySize; i++) {
                if (array[i] <= pivot && array[i] != -1) {
                    count++;
                }
            }

            if (write(childToParent[id][WRITE], &count, sizeof(count)) == -1) {
                perror("Child write to parent failed");
                exit(EXIT_FAILURE);
            }


        } else if (command == SMALL) { //CHECK
            printf("Child %d: Handling SMALL, updating array.\n", id);

            // Respond to SMALL command
            int temp[arraySize], index = 0;
            for (int i = 0; i < arraySize; i++) {
                if (array[i] > pivot && array[i] != -1) {
                    temp[index++] = array[i];
                }
            }
            if (index == 0) {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    array[i] = -1; // Mark array as empty
                }
            } else {
                arraySize = index; // Update the size of the array
                memcpy(array, temp, sizeof(int) * index);
            }
            
        } else if (command == LARGE) {  //CHECK   
            printf("Child %d: Handling LARGE, updating array.\n", id);

            // Respond to LARGE command
            int temp[arraySize], index = 0;
            for (int i = 0; i < arraySize; i++) {
                if (array[i] < pivot && array[i] != -1) {
                    temp[index++] = array[i];
                }
            }
            if (index == 0) {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    array[i] = -1; // Mark array as empty
                }
            } else {
                arraySize = index; // Update the size of the array
                memcpy(array, temp, sizeof(int) * index);
            }
        } else if (command == STOP) {
            printf("Child %d: Exiting on STOP command.\n", id);
            exit(0); // Exit when STOP command is received
        }
    }
}

void ParentProcess(int kVal) {
    printf("Parent:Kval is %d", kVal);
    printf("Parent: Waiting for READY signal from all children.\n");//Works!

    // Wait for READY message from all child processes
    int message;
    for (int i = 0; i < NUM_CHILDREN; i++) {
        readResponse(childToParent[i][READ], &message);
        if (message != READY) {
            fprintf(stderr, "Parent: Error, expected READY message from child %d, received %d\n", i, message);
            exit(EXIT_FAILURE);
        }
        printf("Parent: Received READY from Child %d\n", i); //WORKS!
    }

    int kthValue = -1; // Placeholder for the kth smallest value, initialized to an invalid state

    while (kthValue == -1) {
        int pivotIndex = rand() % NUM_CHILDREN; // Select a random child for pivot
        printf("Parent: Randomly selecting Child %d\n", pivotIndex); //WORKS!
        writeCommand(parentToChild[pivotIndex][WRITE], REQUEST);

        int pivotValue;
        readResponse(childToParent[pivotIndex][READ], &pivotValue); // reading response on request from ch
        
        while (pivotValue == -1) {
            pivotIndex = rand() % NUM_CHILDREN; // Select a random child for pivot
            printf("Parent: Randomly selecting Child %d\n", pivotIndex); //WORKS!
            writeCommand(parentToChild[pivotIndex][WRITE], REQUEST);
            readResponse(childToParent[pivotIndex][READ], &pivotValue);
        }

        printf("Parent: Selected Pivot Value %d from Child %d\n", pivotValue, pivotIndex); //WORKS!

        int lessThanPivot = 0;
        for (int i = 0; i < NUM_CHILDREN; i++) {
            printf("Parent: Sending PIVOT %d to Child %d\n", pivotValue, i); //WORKS!
            writeCommand(parentToChild[i][WRITE], PIVOT);
            write(parentToChild[i][WRITE], &pivotValue, sizeof(pivotValue));

            int count;
            readResponse(childToParent[i][READ], &count);
            lessThanPivot += count;
            printf("Parent: Child %d reported %d elements less than pivot\n", i, count); //WORKS!
        }

        printf("Parent: Total elements less than pivot: %d\n", lessThanPivot); //WORKS!

        printf("Parent:Kval is %d", kVal);
        if (lessThanPivot == (kVal - 1)){
            printf("Parent: Found kth smallest value: %d\n", pivotValue);
            kthValue = pivotValue;
            break;
        } 
        if (kVal > lessThanPivot) {
            kVal -= lessThanPivot; // Adjust kVal
            for (int i = 0; i < NUM_CHILDREN; i++) {
                writeCommand(parentToChild[i][WRITE], SMALL);
            }
            printf("Parent: Removing smaller elements\n");
        } else {
            for (int i = 0; i < NUM_CHILDREN; i++) {
                writeCommand(parentToChild[i][WRITE], LARGE);
            }
            printf("Parent: Removing larger elements, adjusting kVal to %d\n", kVal);//WORKS!
        }
        printf("Parent: kVal: %d\n", kVal); //WORKS!
    }

    // Notify children to stop
    printf("Parent: Sending STOP to all children.\n");
    for (int i = 0; i < NUM_CHILDREN; i++) {
        writeCommand(parentToChild[i][WRITE], STOP);
    }

    printf("Parent: kth smallest value is %d \n search completed.\n", kthValue);
}

int main(void) {
    srand((unsigned int)time(NULL));
    // Initialize random seed
    initializePipes();

    pid_t pid;
    printf("Main: Forking child processes.\n"); //WORKS!
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid = fork();
        if (pid == 0) { // Child process
            ChildProcess(i);
            exit(0);
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        printf("Main: Child %d started with PID %d.\n", i, pid); //WORKS!
    }

    printf("Main: Parent process starting.\n"); //WORKS!
    // Parent process: Close the unused ends correctly
    for (int i = 0; i < NUM_CHILDREN; i++) {
        close(parentToChild[i][READ]);
        close(childToParent[i][WRITE]);
    }

    ParentProcess(13); // Example: Looking for the 13th smallest element

    // Wait for all children to exit
    while (wait(NULL) > 0);

    printf("Main: All children have exited. Program complete.\n");

    return 0;
}
