// Library Imports:
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

// Constants:
#define TO_BRIDGER 0
#define TO_BOZEMAN 1


/// Global varibales:

static int max_cars;

static int cars_crossed;
pthread_mutex_t crossed_count_lock;
pthread_cond_t crossed;

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
    int direction;
};

typedef struct car_generator car_generator_t;

// Vehicle simulation

// NOTE: represent each vehicle as a thread.
void* OneVehicle(void* arg){
    
    int car_id;

    // Lock the car generator value to be sure other threads don't get the same id value.
    pthread_mutex_lock(&(*(car_generator_t*)(arg)).mutex);
    car_id = (*(car_generator_t*)(arg)).id;
    // TODO: Make this generate from SEED value.
    int direction = (*(car_generator_t*)(arg)).direction;
    pthread_mutex_unlock(&(*(car_generator_t*)(arg)).mutex);
    pthread_cond_signal(&(*(car_generator_t*)(arg)).done);

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

    // Set up for mutex time out
    struct timespec timeToWait;
    struct timeval now;
    int rc;

    gettimeofday(&now, NULL);
    // For waiting 5 seconds.
    timeToWait.tv_sec = now.tv_sec + 1;
    timeToWait.tv_nsec = (now.tv_usec*1000UL);
    rc = 0;

    // Checks if it is safe for the car to go on the one way
    pthread_mutex_lock(&one_way_t);

    printf("Car %d: current direction: %d == direction: %d\n", car_id, current_direction, direction);
    while((Cars_On_OneWay >= max_cars) || (current_direction != direction)){
        printf("Car %d In this while loop\n", car_id);
        rc = pthread_cond_timedwait(&one_way_vals_free, &one_way_t, &timeToWait);

        printf("RC: %d\n", rc);
        // Do Direction function if rc value is true?
        if(rc == 110 && Cars_On_OneWay < 1){
            
            current_direction = (current_direction+1)%2;
            printf("In Car: New direction is now: %d\n", current_direction);
        }
    }

    // Puts a car on the one way.
    printf("Putting Car: %d on the one way, going towards: %d\n\n", car_id,direction);
    Cars_On_OneWay += 1;
    pthread_mutex_unlock(&one_way_t);

    pthread_cond_signal(&one_way_vals_free);
}

// Outputs the car's state as it passes through the one way.
int OnBridgerOneWay(int* car_id, int direction){

    char dir[7];
    if(direction == TO_BOZEMAN){
        strcpy(dir, "BOZEMAN");
    }else{
        strcpy(dir, "BRIDGER");
    }

    printf("Car %d is on the run way going towards %s\n", car_id, dir);
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

    // Changes direction by default if the amount of cars on the one way is 0 after this car exits.
    if(Cars_On_OneWay < 1){
        current_direction = (current_direction+1)%2;
        printf("No more on one-way after exit, direction is now: %d\n", current_direction);
    }
    pthread_mutex_unlock(&one_way_t);

    pthread_mutex_lock(&crossed_count_lock);
    cars_crossed++;
    printf("CARS CROSSED: %d\n", cars_crossed);
    pthread_mutex_unlock(&crossed_count_lock);
    pthread_cond_signal(&crossed);
}

// Main Program Loop
int main(int argc, char* argv[]){
    // Initializes iterator
    int iter;
    int thread_count;
    int num_cars;
    int seed;
    cars_crossed = 0;

    // Helps with generating car ids.
    car_generator_t car_generator;
    pthread_mutex_init (&car_generator.mutex, NULL);
    pthread_cond_init (&car_generator.done, NULL);

    if( argc < 4){
        // Set up values to a feault configuration rather than specified thread count/car max.
        thread_count = 10;
        max_cars = 3;
        num_cars = 15;
        // Default run has a random seed.
        seed = time(NULL);
        printf("Not enough arguements, using default values.\n");
    } else{
        // Initializes the maximum about of threads allowed.
        thread_count = atoi(argv[1]);

        // initializes the maximum threshold of cars the one way can handle at a time.
        max_cars = atoi(argv[2]);
        if(max_cars < 1){
            printf("Max_Cars value cannot be below 1. Defaulting value to: 1.\n");
            max_cars = 1;
        }

        // Initializes the number of cars to generate
        num_cars = atoi(argv[3]);
        if(num_cars < 1){
            printf("The number of Cars cannot be below 1. Defaulting value to: 1.\n");
            num_cars = 1;
        }

        // Sets the random seed value for generating car's directions.
        seed = atoi(argv[4]);
    }
    
        srand(seed);

        printf("SIMULATION SETTINGS:\n");

        printf("1) Initializing with %d threads.\n", thread_count);
        printf("2) Maximum # of cars allowed on the one-way: %d\n", max_cars);

        // Create the threads
        pthread_t threads[thread_count];

        // Lock the mutex for car generator
        pthread_mutex_lock(&car_generator.mutex);



        for(iter = 0; iter < thread_count; iter++){
            // Checks the return value of the thread creation to make sure it was able to initialize sucessfully.

            // Changes the current car generator id value
            car_generator.id = iter;
            // randomly generates from seeded value.
            car_generator.direction = rand() % 2;

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

        printf("TOTAL CARS CROSSED: %d\n", cars_crossed);
        return 0;
}