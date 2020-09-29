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
#include <signal.h>
#include "SortedList.h"

int nthreads = 1;
int iterations = 1;
int yieldFlag = 0;
int nElements = 0;
int opt_yield = 0;
char syncOp;
int spin;
SortedList_t* list;
SortedListElement_t* elements;
pthread_mutex_t mutex;

void signalHandler(int signal){
    if(signal == SIGSEGV){
        fprintf(stderr, "Segmentation Fault: %s\n", strerror(errno));
        exit(2);
    }
}

void initializeList(){
    list = (SortedList_t*) malloc(sizeof(SortedList_t));
    if(!list){
        fprintf(stderr, "Error with initializing list: %s\n", strerror(errno));
    }
    list->key = NULL;
    list->prev = list;
    list->next = list;
}

void setkeys(){
    srand((unsigned int) time(NULL));
    char a[] = "abcdefghijklmnopqrstuvwxyz";
    int i;
    char generated[2];
    for(i = 0; i < nElements; i++){
        generated[0] = a[rand()%26];
        generated[1] = '\0';
        elements[i].key = generated;
    } 
}

void printLine(int numThreads, int numIterations, int numLists, int numOps, long long runTime, long long avgTime){
    fprintf(stdout, ",%d,%d,%d,%d,%lld,%lld\n", numThreads, numIterations, numLists, numOps, runTime, avgTime);
}

void* threadMain(void* arr){
    int val = *((int*)arr);
    int i = 0;
    //insert
    for(i = val*iterations; i < (val+1)*iterations; i++){
        if(syncOp == 'm'){
            pthread_mutex_lock(&mutex);
            SortedList_insert(list, &elements[i]); 
            pthread_mutex_unlock(&mutex);
        }
        else if(syncOp == 's'){
            while (__sync_lock_test_and_set(&spin, 1))
                continue;  
            SortedList_insert(list, &elements[i]);
            __sync_lock_release(&spin);
        }
        else{
            SortedList_insert(list, &elements[i]);
        }
    }

    //get list length
    int length;
    if(syncOp == 'm'){
        pthread_mutex_lock(&mutex);
        length = SortedList_length(list);
        pthread_mutex_unlock(&mutex);
    }
    else if(syncOp == 's'){
        while (__sync_lock_test_and_set(&spin, 1))
            continue;  
        length = SortedList_length(list);
        __sync_lock_release(&spin);
    }
    else{
        length = SortedList_length(list);
    }
    /*
    if(length < iterations){
        fprintf(stderr, "Error with list length(less than iterations): %s\n", strerror(errno));
        exit(2);
    }
    */
    if(length < 0){
        fprintf(stderr, "Error with list length(less than zero): %s\n", strerror(errno));
        exit(2);
    }

    //delete all inserted keys
    for(i = val * iterations; i < (val+1)*iterations; i++){
        if(syncOp == 'm'){
            pthread_mutex_lock(&mutex);
            SortedListElement_t* theChosenOne = SortedList_lookup(list, elements[i].key);
            if(theChosenOne == NULL){
                fprintf(stderr, "Error with looking up element: %s\n", strerror(errno));
                exit(2);
            }
            int stat = SortedList_delete(theChosenOne);
            if(stat != 0){
                fprintf(stderr, "Error with deleted element: %s\n", strerror(errno));
                exit(2);
            }
            pthread_mutex_unlock(&mutex);
        }
        else if(syncOp == 's'){
            while (__sync_lock_test_and_set(&spin, 1))
                continue;  
            SortedListElement_t* theChosenOne = SortedList_lookup(list, elements[i].key);
            if(theChosenOne == NULL){
                fprintf(stderr, "Error with looking up element: %s\n", strerror(errno));
                exit(2);
            }
            int stat = SortedList_delete(theChosenOne);
            if(stat != 0){
                fprintf(stderr, "Error with deleted element: %s\n", strerror(errno));
                exit(2);
            }
            __sync_lock_release(&spin);
        }
        else{
            SortedListElement_t *theChosenOne = SortedList_lookup(list, elements[i].key);
            if(theChosenOne == NULL){
                fprintf(stderr, "Error with looking up element: %s\n", strerror(errno));
                exit(2);
            }
            int stat = SortedList_delete(theChosenOne);
            if(stat != 0){
                fprintf(stderr, "Error with deleted element: %s\n", strerror(errno));
                exit(2);
            }
        }
    }
    return NULL;
}

int main(int argc, char* argv[]){
    struct option ops[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
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
                yieldFlag = 1;
                size_t i;
                for(i  = 0; i < strlen(optarg); i++){
                    switch(optarg[i]){
                        case 'i':
                            opt_yield |= INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield |= DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield |= LOOKUP_YIELD;
                            break;
                        default:
                            fprintf(stderr, "Error with yield option: %s\n", strerror(errno));
                            exit(1);
                    }
                }
                break;
                
            case 's':
                syncOp = optarg[0];
                if (syncOp == 'm') {
		            if (pthread_mutex_init(&mutex, NULL) != 0){
			            fprintf(stderr, "Error with pthread_mutex_init: %s\n", strerror(errno));
			            exit(1);
		            }	
	            }
                if (syncOp == 's'){
                    spin = 0;
                }
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument: %s\n", strerror(errno));
                exit(1);
                break;
        }
    }
    signal(SIGSEGV, signalHandler);
    nElements = nthreads * iterations;
    initializeList();
    elements = (SortedListElement_t*) malloc(nElements * sizeof(SortedListElement_t));
    if(elements == NULL){
        fprintf(stderr, "Error with elements memory allocations: %s\n", strerror(errno));
        exit(2);
    }
    setkeys();
    pthread_t* threads = malloc(nthreads * sizeof(pthread_t));
    struct timespec start, end;
    int stat = clock_gettime(CLOCK_MONOTONIC, &start);
    if(stat < 0){
        fprintf(stderr, "Error with clock_gettime start: %s\n", strerror(errno));
        exit(1);
    }
    int num[nthreads];
    int i;
    for (i = 0; i < nthreads; i++){
        num[i] = i;
        stat = pthread_create(&threads[i], NULL, &threadMain, &num[i]);
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
    if(SortedList_length(list) != 0){
        fprintf(stderr, "Error with completely clearing list: %s\n", strerror(errno));
        exit(2);
    }
    //print to csv stuff
    int numLists = 1;
    long long diff = end.tv_sec - start.tv_sec;
    long long diffn = end.tv_nsec - start.tv_nsec;
    long long runTime = (1000000000L * diff) + diffn;
    int numOps = nthreads * iterations * 3;
    long long avgTime = runTime/numOps;
    fprintf(stdout, "list-");
    if (opt_yield) {
        if (opt_yield & INSERT_YIELD)
            fprintf(stdout, "i");
        if (opt_yield & DELETE_YIELD)
            fprintf(stdout, "d");
        if (opt_yield & LOOKUP_YIELD)
            fprintf(stdout, "l");
    }
    else{
        fprintf(stdout, "none");
    }
    if(syncOp == 's')
        fprintf(stdout, "-s");
    else if(syncOp == 'm')
        fprintf(stdout, "-m");
    else
        fprintf(stdout, "-none");
    
    printLine(nthreads, iterations, numLists, numOps, runTime, avgTime);
    free(threads);
    free(elements);
    free(list);
    exit(0);
    return 0;
}