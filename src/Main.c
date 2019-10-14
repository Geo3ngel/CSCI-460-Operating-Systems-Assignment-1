// Library Imports:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Constants:
#define MAXCARS 3

// Global varibales:

// Main Program Loop

// Vehicle simulation


OneVehicle(direction){
    ArriveBridgerOneWay(direction);
    //now the car is on the one-way section!

    OnBridgerOneWay(direction);
    ExitBridgerOneWay(direction);

    //now the car is off the one way.
}
