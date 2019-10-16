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
#define WAITFOR .01

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

/// Global varibales:

static int max_cars;
static int dir_change_limit;
static bool draining_one_way;

// Variables pertaining to the total amount of cars that have crossed the one way.
static int cars_crossed;
pthread_mutex_t crossed_count_lock;
pthread_cond_t crossed;

// Mutex for managing one way related values below.
pthread_mutex_t one_way_t;
// Notifier for when the below values are freed
pthread_cond_t one_way_vals_free;
static int dir_change_count;

pthread_mutex_t direction_change;
pthread_cond_t direction_changed;

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

// Translates a car's direction to string.
char* DirectionToString(int direction){
    char dir[7];
    if(direction == TO_BOZEMAN){
        strcpy(dir, "BOZEMAN");
    }else{
        strcpy(dir, "BRIDGER");
    }

    char * rtn_ptr = dir;
    return rtn_ptr;
}

// Represents a vehicle as a thread
void* OneVehicle(void* arg){
    
    int car_id;

    // Lock the car generator value to be sure other threads don't get the same id value.
    pthread_mutex_lock(&(*(car_generator_t*)(arg)).mutex);
    car_id = (*(car_generator_t*)(arg)).id;
    // TODO: Make this generate from SEED value.
    int direction = (*(car_generator_t*)(arg)).direction;
    pthread_mutex_unlock(&(*(car_generator_t*)(arg)).mutex);
    pthread_cond_signal(&(*(car_generator_t*)(arg)).done);

    printf("New CAR: %d. going in direction: %s\n", car_id, DirectionToString(direction));

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
    
    rc = 0;

    // Checks if it is safe for the car to go on the one way
    pthread_mutex_lock(&one_way_t);

    printf("Car %d has arrived at the one way facing %s\n", car_id, DirectionToString(direction));
    while((Cars_On_OneWay >= max_cars) || (current_direction != direction) || draining_one_way){
        timeToWait.tv_sec = now.tv_sec + WAITFOR;
        timeToWait.tv_nsec = (now.tv_usec*1000UL);
        //printf("Car %d is currently waiting to get on the One-Way towards %s\n", car_id, DirectionToString(direction));

        // If the time out occurs, check if direction change is possible.
        if(rc == 110){
            if(Cars_On_OneWay < 1){
                draining_one_way = false;
                current_direction = (current_direction+1)%2;
                printf("Direction has been changed to: %s\n", 
                DirectionToString(current_direction));
            } else if(dir_change_count >= dir_change_limit){
                
                draining_one_way = true;

                // Reset the amount of cars for direction change counter
                dir_change_count = 0;
            }
        }

        rc = pthread_cond_timedwait(&one_way_vals_free, &one_way_t, &timeToWait);
    }
    
    pthread_mutex_lock(&direction_change);
    dir_change_count++;
    pthread_mutex_unlock(&direction_change);
    pthread_cond_signal(&direction_changed);

    // Puts a car on the one way.
    printf("Putting Car: %d on the one way, going towards: %s\n\n", car_id, DirectionToString(direction));
    Cars_On_OneWay++;
    pthread_mutex_unlock(&one_way_t);

    pthread_cond_signal(&one_way_vals_free);
}

// Outputs the car's state as it passes through the one way.
int OnBridgerOneWay(int* car_id, int direction){

    printf("Car %d is on the run way going towards %s \n", car_id, DirectionToString(direction));
}

// Removes the car from the one way
int ExitBridgerOneWay(int car_id, int direction){
        
    // Checks if it is safe for the car to go on the one way
    pthread_mutex_lock(&one_way_t);

    // Puts a car on the one way.
    printf(KYEL "Car: %d has left the one way, and is continuing towards: %s\n" KWHT, car_id, DirectionToString(direction));
    Cars_On_OneWay -= 1;

    // TODO: change direction & notify other cars if there are none left
    printf("Cars_On_OneWay: %d\n\n", Cars_On_OneWay);

    // Changes direction by default if the amount of cars on the one way is 0 after this car exits.
    pthread_mutex_unlock(&one_way_t);

    pthread_mutex_lock(&crossed_count_lock);
    cars_crossed++;
    // TODO: Remove. printf("CARS CROSSED: %d\n", cars_crossed);
    pthread_mutex_unlock(&crossed_count_lock);
    pthread_cond_signal(&crossed);

    // Wake up threads trying to change direction
    pthread_cond_signal(&direction_changed);

}

// Main Program Loop
int main(int argc, char* argv[]){
    // Initializes iterator
    int iter;
    int num_cars;
    int dir_change_limit;
    int seed;
    cars_crossed = 0;
    dir_change_count = 0;
    draining_one_way = false;

    // Helps with generating car ids.
    car_generator_t car_generator;
    pthread_mutex_init (&car_generator.mutex, NULL);
    pthread_cond_init (&car_generator.done, NULL);

    if( argc < 4){
        // Set up values to a feault configuration rather than specified thread count/car max.
        num_cars = 100;
        max_cars = 5;
        dir_change_limit = 15;
        // Default run has a random seed.
        seed = time(NULL);
        printf("Not enough arguements, using default values.\n");
    } else{
        // Initializes the maximum about of threads allowed.
        num_cars = atoi(argv[1]);
        if(num_cars < 1){
            printf("The number of Cars cannot be below 1. Defaulting value to: 1.\n");
            num_cars = 1;
        }

        // initializes the maximum threshold of cars the one way can handle at a time.
        max_cars = atoi(argv[2]);
        if(max_cars < 1){
            printf("Max_Cars value cannot be below 1. Defaulting value to: 1.\n");
            max_cars = 1;
        }

        // Initializes the number of cars to generate
        dir_change_limit = atoi(argv[3]);
        if(dir_change_limit < 1){
            printf("The limit of Cars per direction change cycle can't be less than 1, setting to 1...\n");
            dir_change_limit = 1;
        }

        // Sets the random seed value for generating car's directions.
        seed = atoi(argv[4]);
    }
    
        srand(seed);

        printf("SIMULATION SETTINGS:\n");

        printf("1) Initializing with %d Cars.\n", num_cars);
        printf("2) Maximum # of cars allowed on the one-way: %d\n", max_cars);
        printf("3) Maximum # of cars per one-way cycle: %d\n\n", dir_change_limit);

        // Create the threads
        pthread_t threads[num_cars];

        // Lock the mutex for car generator
        pthread_mutex_lock(&car_generator.mutex);



        for(iter = 0; iter < num_cars; iter++){
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
        for(iter = 0; iter < num_cars; iter++){
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