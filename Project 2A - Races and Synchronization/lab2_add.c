//NAME: William Chen
//EMAIL: billy.lj.chen@gmail.com
//ID: 405131881


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>

int nthreads = 1;
int iterations = 1;
int opt_yield = 0;
int yieldFlag = 0;
char syncOp;
int spin = 0;
char* chosenName;
pthread_mutex_t mutex; 
long long counter;


void addNone(long long *pointer, long long value){
    long long sum = *pointer + value;
    if (opt_yield == 1){
        sched_yield();
    }
    *pointer = sum;
}

void addM(long long *pointer, long long value){
    pthread_mutex_lock(&mutex);
    addNone(pointer, value);
    pthread_mutex_unlock(&mutex);
}

void addS(long long *pointer, long long value){
    while(__sync_lock_test_and_set(&spin, 1));
    addNone(pointer, value);
    __sync_lock_release(&spin);
}

void addC(long long *pointer, long long value){
    long long sum, prev;
    do{
        prev = counter;
        sum = prev + value;
        if(opt_yield == 1){
            sched_yield();
        }
    } while (__sync_val_compare_and_swap(pointer, prev, sum) != prev);
}

void printLine(char* name, int numThreads, int numIterations, int numOps, long long runTime, long long avgTime, long long total){
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", name, numThreads, numIterations, numOps, runTime, avgTime, total);
}

void* addMain(void* ptr){
    int i;
    for(i = 0; i < iterations; i++){
        switch(syncOp){
            case 'm':
                addM(&counter, 1);
                addM(&counter, -1);
                break;
            case 's':
                addS(&counter, 1);
                addS(&counter, -1);
                break;
            case 'c':
                addC(&counter, 1);
                addC(&counter, -1);
                break;
            default:
                addNone(&counter, 1);
                addNone(&counter, -1);
                break;
        }
    }
    return ptr;
}

void chooseName(){
    if(!opt_yield && syncOp == 'm'){
        chosenName = "add-m";
    }
    else if(!opt_yield && syncOp == 's'){
        chosenName = "add-s";
    }
    else if(!opt_yield && syncOp == 'c'){
        chosenName = "add-c";
    }
    else if(opt_yield && syncOp == 'm'){
        chosenName = "add-yield-m";
    }
    else if(opt_yield && syncOp == 's'){
        chosenName = "add-yield-s";
    }
    else if(opt_yield && syncOp == 'c'){
        chosenName = "add-yield-c";
    }
    else if(opt_yield){
        chosenName = "add-yield-none";
    }
    else{
        chosenName = "add-none";
    }
}
int main(int argc, char* argv[]){
    struct option ops[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", no_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {0, 0, 0, 0}
    };

    int c;
    while(1){
        c = getopt_long(argc, argv, "", ops, NULL);
        if(c == -1)
        break;
        switch(c){
            case 't':
                nthreads = atoi(optarg);
                break;
            case 'i':
                iterations = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                syncOp = optarg[0];
                yieldFlag = 1;
                if (syncOp == 'm') {
		            if (pthread_mutex_init(&mutex, NULL) != 0){
			            fprintf(stderr, "Error with pthread_mutex_init: %s\n", strerror(errno));
			            exit(1);
		            }	
	            }
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument: %s\n", strerror(errno));
                exit(1);
                break;
        }
    }
    counter = 0;
    struct timespec start, end;
    int stat = clock_gettime(CLOCK_MONOTONIC, &start);
    if(stat < 0){
        fprintf(stderr, "Error with clock_gettime start: %s\n", strerror(errno));
        exit(1);
    }
    pthread_t* threads = (pthread_t*) malloc(nthreads * sizeof(pthread_t));
    int i;
    for (i = 0; i < nthreads; i++){
        stat = pthread_create(&threads[i], NULL, &addMain, NULL);
        if(stat < 0){
            fprintf(stderr, "Error with pthread_create: %s\n", strerror(errno));
            exit(1);
        }
    }
    for (i = 0; i < nthreads; i++){
        stat = pthread_join(threads[i], NULL);
        if(stat < 0){
            fprintf(stderr, "Error with pthread_join: %s\n", strerror(errno));
            exit(1);
        }
    }
    stat = clock_gettime(CLOCK_MONOTONIC, &end);
    if(stat < 0){
        fprintf(stderr, "Error with clock_gettime end: %s\n", strerror(errno));
        exit(1);
    }

    long long diff = end.tv_sec - start.tv_sec;
    long long diffn = end.tv_nsec - start.tv_nsec;
    long long runTime = (1000000000L * diff) + diffn;
    int numOps = nthreads * iterations * 2;
    long long avgTime = runTime/numOps;
    chooseName();
    printLine(chosenName, nthreads, iterations, numOps, runTime, avgTime, counter);
    free(threads);
    if(syncOp == 'm'){
        pthread_mutex_destroy(&mutex);
    }
    exit(0);
}