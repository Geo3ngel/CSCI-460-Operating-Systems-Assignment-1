// Library Imports:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Constants:
#define TO_BRIDGER 0
#define TO_BOZEMAN 1


// Global varibales:

_Atomic int* Cars_On_OneWay;

// Main Program Loop
int main(int argc, char* argv[]){
    // Initializes iterator
    int iter = 0;
    int thread_count;
    int max_cars;

    if( argc < 2){
        // Set up values to a feault configuration rather than specified thread count/car max.
        thread_count = 10;
        max_cars = 3;
    } else{
        // Initializes the maximum about of threads allowed.
        thread_count = atoi(argv[1]);

        // initializes the maximum threshold of cars the one way can handle at a time.
        max_cars = atoi(argv[2]);
    }
    
        printf("Initializing with %d threads.\n", thread_count);

        printf("%d is the maximum amount of cars allowed on the one-way.\n", max_cars);
}
// Vehicle simulation

// NOTE: represent each vehicle as a thread.
void* OneVehicle(void* direction){
    ArriveBridgerOneWay(direction);
    //now the car is on the one-way section!

    OnBridgerOneWay(direction);
    ExitBridgerOneWay(direction);

    //now the car is off the one way.
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