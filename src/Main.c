// Library Imports:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

// Constants:
#define TO_BRIDGER 0
#define TO_BOZEMAN 1


// Global varibales:

_Atomic int* Cars_On_OneWay;

// Vehicle simulation

// NOTE: represent each vehicle as a thread.
void* OneVehicle(void* direction){
    
    ArriveBridgerOneWay(direction);
    //now the car is on the one-way section!

    OnBridgerOneWay(direction);
    ExitBridgerOneWay(direction);

    //now the car is off the one way.
    printf("NEW CAR\n");
}

// Puts a car on the one-way once it is confirmed to be safe.
int ArriveBridgerOneWay(void* direction){
// TODO: Must not return until its safe for the car to get on the one-way

// Checks if it is safe for the car to go on the one way

// Sends a car on the one-way in the direction specified

}

// Outputs the car's state as it passes through the one way.
int OnBridgerOneWay(void* direction){
// TODO: Use prints or graphics library?

// Checks the capacity of the one way

// Monitors for race conditions via visual output!
}

// Removes the car from the one way
int ExitBridgerOneWay(void* direction){
// Removes car from critical area?

// Unlocks access to this area?

}

// Main Program Loop
int main(int argc, char* argv[]){
    // Initializes iterator
    int iter = 0;
    int thread_count;
    int max_cars;
    int num_cars;

    if( argc < 3){
        // Set up values to a feault configuration rather than specified thread count/car max.
        thread_count = 10;
        max_cars = 3;
        num_cars = 15;
        printf("Not enough arguements, using default values.");
    } else{
        // Initializes the maximum about of threads allowed.
        thread_count = atoi(argv[1]);

        // initializes the maximum threshold of cars the one way can handle at a time.
        max_cars = atoi(argv[2]);

        // Initializes the number of cars to generate
        num_cars = atoi(argv[3]);
    }
    
        printf("SIMULATION SETTINGS:\n");

        printf("1) Initializing with %d threads.\n", thread_count);
        printf("2) Maximum # of cars allowed on the one-way: %d\n", max_cars);

        // Create the threads
        pthread_t threads[thread_count];

        for(iter = 0; iter < thread_count; iter++){
            // Checks the return value of the thread creation to make sure it was able to initialize sucessfully.
            if(pthread_create(&threads[iter], NULL, OneVehicle, (void *) &iter)){
                printf("ERROR CREATING THREAD: Assuming not enough threads avalible.");
                exit(-1);
            }
        }

        sleep(1);

        printf("Finished making threads.\n");

        iter = 0;

        // Join all the threads
        for(iter = 0; iter < thread_count; iter++){
            // Checks that all threads are able to join
            if(pthread_join(threads[iter], NULL)){
                printf("ERROR Joining threads.");
            }
        }
        printf("Finished Joining Threads.\n");

}