// Library Imports:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Constants:
#define MAXCARS 3

#define TO_BRIDGER 0
#define TO_BOZEMAN 1


// Global varibales:

// Main Program Loop

// Vehicle simulation

// NOTE: represent each vehicle as a thread.
OneVehicle(direction){
    ArriveBridgerOneWay(direction);
    //now the car is on the one-way section!

    OnBridgerOneWay(direction);
    ExitBridgerOneWay(direction);

    //now the car is off the one way.
}

// Puts a car on the one-way once it is confirmed to be safe.
void* ArriveBridgerOneWay(void* direction){
// TODO: Must not return until its safe for the car to get on the one-way
// Checks if it is safe for the car to go on the one way

// Sends a car on the one-way in the direction specified

}

// Outputs the car's state as it passes through the one way.
void* OnBridgerOneWay(void* direction){
// TODO: Use prints or graphics library?
}