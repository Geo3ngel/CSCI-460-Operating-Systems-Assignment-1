// Library Imports:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

// Constants:
#define TO_BRIDGER 0
#define TO_BOZEMAN 1


/// Global varibales:

static int max_cars;

// Mutex for cars on one way
pthread_mutex_t oneway_load;
// Condition for cars to be able to be added to one way
pthread_cond_t under_max_load;
// One way current cars counter
static int Cars_On_OneWay = 0;


// The current Direction cars entering the one way are heading.
static volatile int current_direction = TO_BRIDGER;
// Mutex for the current direction
pthread_mutex_t direction_lock;


// Vehicle simulation

// NOTE: represent each vehicle as a thread.
void* OneVehicle(int* id){
    
    printf("NEW CAR: %d\n", *id);
    int* car_id = id;
    int direction = rand() % 2;
    printf("DIRECTION: %d\n", direction);

    ArriveBridgerOneWay(car_id, 0);
    //now the car is on the one-way section!

    OnBridgerOneWay(direction);
    ExitBridgerOneWay(direction);

    //now the car is off the one way.
    pthread_exit(NULL);
}

// Puts a car on the one-way once it is confirmed to be safe.
int ArriveBridgerOneWay(int* car_id, int direction){

// Checks if it is safe for the car to go on the one way
pthread_mutex_lock(&oneway_load);
pthread_mutex_lock(&direction_lock);

printf("current direction: %d == direction: %d\n", current_direction, direction);
while((Cars_On_OneWay >= max_cars) || (current_direction != direction)){
    printf("In this while loop\n");
    pthread_cond_wait(&under_max_load, &oneway_load);
}

// Puts a car on the one way.
printf("Putting Car: %d on the one way, going towards: %d\n", *car_id,direction);
Cars_On_OneWay += 1;

pthread_mutex_unlock(&oneway_load);
pthread_mutex_unlock(&direction_lock);
}

// Outputs the car's state as it passes through the one way.
int OnBridgerOneWay(int* car_id, int direction){
// TODO: Use prints or graphics library?

// Checks the capacity of the one way

// Monitors for race conditions via visual output!
}

// Removes the car from the one way
int ExitBridgerOneWay(int direction){
// Removes car from critical area?

// Unlocks access to this area?

}

// Main Program Loop
int main(int argc, char* argv[]){
    // Initializes iterator
    int iter = 0;
    int thread_count;
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
            int thread_creation_check = pthread_create(&threads[iter], NULL, OneVehicle, &iter);

            if(thread_creation_check){
                printf("ERROR CREATING THREAD: %d", thread_creation_check);
                exit(-1);
            }
        }

        sleep(.01);

        printf("Finished making threads.\n");

        iter = 0;

        // Join all the threads
        for(iter = 0; iter < thread_count; iter++){
            // Checks that all threads are able to join
            int thread_join_error_check = pthread_join(threads[iter], NULL);
            if(thread_join_error_check){
                printf("ERROR Joining threads.");
            }
        }
        printf("Finished Joining Threads.\n");

}