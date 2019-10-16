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

// Mutex for managing one way related values below.
pthread_mutex_t one_way_t;
// Notifier for when the below values are freed
pthread_cond_t one_way_vals_free;
// Notifier for when a car leaves the one way to check if the direction needs to be updated.
pthread_cond_t car_left_one_way;

// One way current cars counter
static int Cars_On_OneWay = 0;
// The current Direction cars entering the one way are heading.
static volatile int current_direction = TO_BRIDGER;

// TODO: Define other mutex locks and stuff as structs???
struct car_generator {
    pthread_mutex_t mutex;
    pthread_cond_t done;
    int id;
};

typedef struct car_generator car_generator_t;

// Vehicle simulation

// NOTE: represent each vehicle as a thread.
void* OneVehicle(void* arg){
    
    int car_id;

    // Lock the car generator value to be sure other threads don't get the same id value.
    pthread_mutex_lock(&(*(car_generator_t*)(arg)).mutex);
    car_id = (*(car_generator_t*)(arg)).id;
    pthread_mutex_unlock(&(*(car_generator_t*)(arg)).mutex);
    pthread_cond_signal(&(*(car_generator_t*)(arg)).done);

    // TODO: Make this generate from SEED value.
    int direction = rand() % 2;
    printf("New CAR: %d. going in direction: %d\n", car_id, direction);

    ArriveBridgerOneWay(car_id, direction);
    //now the car is on the one-way section!

    //temp 
    sleep(.05);
    OnBridgerOneWay(car_id, direction);
    // temp
    sleep(.05);
    ExitBridgerOneWay(car_id, direction);

    //now the car is off the one way.
    pthread_exit(NULL);
}

// Puts a car on the one-way once it is confirmed to be safe.
int ArriveBridgerOneWay(int car_id, int direction){

    // Checks if it is safe for the car to go on the one way
    pthread_mutex_lock(&one_way_t);

    printf("Car %d: current direction: %d == direction: %d\n", car_id, current_direction, direction);
    while((Cars_On_OneWay >= max_cars) || (current_direction != direction)){
        printf("Car %d In this while loop\n", car_id);
        pthread_cond_wait(&one_way_vals_free, &one_way_t);
    }

    // Puts a car on the one way.
    printf("Putting Car: %d on the one way, going towards: %d\n\n", car_id,direction);
    Cars_On_OneWay += 1;
    pthread_mutex_unlock(&one_way_t);

    pthread_cond_signal(&one_way_vals_free);
}

// Outputs the car's state as it passes through the one way.
int OnBridgerOneWay(int* car_id, int direction){

    printf("Car %d is on the run way going %d\n", car_id, direction);

    // TODO: Make sure cars don't try to pass each other on the one way? (Race conditions)

    // Monitors for race conditions via visual output!
    // TODO: Use prints or graphics library?
}

// Removes the car from the one way
int ExitBridgerOneWay(int car_id, int direction){
        
    // Checks if it is safe for the car to go on the one way
    pthread_mutex_lock(&one_way_t);

    // Puts a car on the one way.
    printf("Car: %d has left the one way, and is continuing towards: %d\n", car_id, direction);
    Cars_On_OneWay -= 1;

    // TODO: change direction & notify other cars if there are none left
    printf("Cars_On_OneWay: %d\n\n", Cars_On_OneWay);
    pthread_mutex_unlock(&one_way_t);

    DirectionChange();

    pthread_cond_signal(&one_way_vals_free);
    pthread_cond_signal(&car_left_one_way);
}

int DirectionChange(){

    printf("Checking DirectionChange...\n");
    pthread_mutex_lock(&one_way_t);
    while(Cars_On_OneWay > 0){
        // TODO: Maybe use a different condition here? (Only update when a car has left the one way)
        pthread_cond_wait(&car_left_one_way, &one_way_t);
    }

    current_direction = (current_direction+1)%2;
    printf("New direction is now: %d\n", current_direction);
    pthread_mutex_unlock(&one_way_t);

    // Lets the remaining cars know the directin has been changed and is ready to take on new ones.
    pthread_cond_signal(&one_way_vals_free);
}

// Main Program Loop
int main(int argc, char* argv[]){
    // Initializes iterator
    int iter;
    int thread_count;
    int num_cars;

    if( argc < 3){
        // Set up values to a feault configuration rather than specified thread count/car max.
        thread_count = 10;
        max_cars = 3;
        num_cars = 15;
        printf("Not enough arguements, using default values.\n");
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

        // Helps with generating car ids.
        car_generator_t car_generator;
        pthread_mutex_init (&car_generator.mutex, NULL);
        pthread_cond_init (&car_generator.done, NULL);

        // Lock the mutex for car generator
        pthread_mutex_lock(&car_generator.mutex);



        for(iter = 0; iter < thread_count; iter++){
            // Checks the return value of the thread creation to make sure it was able to initialize sucessfully.

            // Changes the current car generator id value
            car_generator.id = iter;

            int thread_creation_check = pthread_create(&threads[iter], NULL, OneVehicle, &car_generator);

            pthread_cond_wait(&car_generator.done, &car_generator.mutex);

            if(thread_creation_check){
                printf("ERROR CREATING THREAD: %d\n", thread_creation_check);
                // Release pthread mutex here???
                exit(-1);
            }
        }

        sleep(.01);

        printf("Finished making threads.\n");

        // Join all the threads
        for(iter = 0; iter < thread_count; iter++){
            // Checks that all threads are able to join
            int thread_join_error_check = pthread_join(threads[iter], NULL);
            if(thread_join_error_check){
                printf("ERROR Joining threads.\n");
            }
        }
        printf("Finished Joining Threads.\n");

        // Cleans up after all the synchronization primitives
        pthread_mutex_destroy(&car_generator.mutex);
        pthread_cond_destroy (&car_generator.done);
        // TODO: clean up other sync primitives.

        return 0;
}