//NAME: William Chen
//EMAIL: billy.lj.chen@gmail.com
//ID: 405131881

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "SortedList.h"

long nthreads = 1;
long iterations = 1;
int yieldFlag = 0;
long nElements  = 0;
long nlists = 1;
char syncOp;
int* spin;
SortedList_t* lists;
SortedListElement_t* elements;
pthread_mutex_t* mutex;

void signalHandler(int signal){
    if(signal == SIGSEGV){
        fprintf(stderr, "Segmentation Fault: %s\n", strerror(errno));
        exit(2);
    }
}

void createSublistsAndLocks(){
    long i;
    for (i = 0; i < nlists; i++){
        lists[i].prev = &lists[i];
        lists[i].next = &lists[i];
        lists[i].key = NULL;
    }
    if(syncOp == 'm'){
        mutex = malloc(sizeof(pthread_mutex_t) * nlists);
        if(mutex == NULL){
            fprintf(stderr, "Error malloc mutex space: %s\n", strerror(errno));
	      	exit(1);
        }
    }
    else if(syncOp == 's'){
        spin = malloc(sizeof(int) * nlists);
        if(spin == NULL){
            fprintf(stderr, "Error malloc spinlock space: %s\n", strerror(errno));
	      	exit(1);
        }
    }
    for(i = 0; i < nlists; i++){
        if(syncOp == 'm'){
            if(pthread_mutex_init(&mutex[i], NULL) < 0){
                fprintf(stderr, "Error with mutex init: %s\n", strerror(errno));
                exit(1);
            }
        }
        else if (syncOp == 's'){
            spin[i] = 0;
        }
    }
}

void setKeys(){
    int i = 0;
    for (; i < nElements; i++)
    {
        char* key = malloc(1);
        if (!key)
            fprintf(stderr, "Error with setKey: %s\n", strerror(errno));
        int up = rand() % 2;
        char let;
        int off = rand() % 26;
        if (up)
            let = 'A' + off;
        else
            let = 'a' + off;
        key[0] = let;
        elements[i].key = key;
    }
}

//simple hash function
unsigned long hash(const char *key){
    int val = (int)*key;
    val = val*71;
	return val % nlists;
}

void* threadMain(void* arr){
    long i;
    SortedListElement_t* listIndex = arr;
    long waitTime = 0;
    struct timespec start, end;
    //insert
    for (i = 0; i < iterations; i++) {
		unsigned long hashVal = hash(listIndex[i].key);
		int stat = clock_gettime(CLOCK_MONOTONIC, &start);
        if(stat < 0){
            fprintf(stderr, "Error with clock_gettime start: %s\n", strerror(errno));
            exit(1);
        }
		if (syncOp == 'm') {
	        pthread_mutex_lock(mutex + hashVal);
	    }
        else if (syncOp == 's') {
	    	while (__sync_lock_test_and_set(spin + hashVal, 1));
	    }
        stat = clock_gettime(CLOCK_MONOTONIC, &end);
        if(stat < 0){
            fprintf(stderr, "Error with clock_gettime end: %s\n", strerror(errno));
            exit(1);
        }
		SortedList_insert(lists + hashVal, (SortedListElement_t *)(listIndex + i));
    	if (syncOp == 'm') {
        	pthread_mutex_unlock(mutex + hashVal);
   		}
        else if (syncOp == 's'){
        	__sync_lock_release(spin + hashVal);
    	}
        waitTime += 1000000000 * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	}
    SortedListElement_t* search;
    i = 0;
    for(; i < iterations; i++){
        int val = hash(listIndex[i].key);
        int stat = clock_gettime(CLOCK_MONOTONIC, &start);
        if(stat < 0){
            fprintf(stderr, "Error with clock_gettime start: %s\n", strerror(errno));
            exit(1);
        }
		if (syncOp == 'm') {
	        pthread_mutex_lock(mutex + val);
	    }
        else if (syncOp == 's') {
	    	while (__sync_lock_test_and_set(spin + val, 1));
	    }
        stat = clock_gettime(CLOCK_MONOTONIC, &end);
        if(stat < 0){
            fprintf(stderr, "Error with clock_gettime end: %s\n", strerror(errno));
            exit(1);
        }

        search = SortedList_lookup(lists + val, listIndex[i].key);
        if(search == NULL){
            fprintf(stderr, "Error with looking up element: %s\n", strerror(errno));
            exit(2);
        }
        stat = SortedList_delete(search);
        if(stat != 0){
            fprintf(stderr, "Error with deleted element: %s\n", strerror(errno));
            exit(2);
        }

        if (syncOp == 'm') {
        	pthread_mutex_unlock(mutex + val);
   		} 
        else if (syncOp == 's'){
        	__sync_lock_release(spin + val);
    	}
        waitTime += 1000000000 * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    }
    return (void*)waitTime;

}

void printLine(int numThreads, int numIterations, int numLists, long numOps, long runTime, long avgTime, long waitLock){
    fprintf(stdout, ",%d,%d,%d,%ld,%ld,%ld,%ld\n", numThreads, numIterations, numLists, numOps, runTime, avgTime, waitLock);
}

int main(int argc, char* argv[]){
    struct option ops[] = {
        {"threads", required_argument, NULL, 't'},
        {"iterations", required_argument, NULL, 'i'},
        {"yield", required_argument, NULL, 'y'},
        {"sync", required_argument, NULL, 's'},
        {"lists", required_argument, NULL, 'l'},
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
                break;
            case 'l':
                nlists = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Error: Invalid Argument: %s\n", strerror(errno));
                exit(1);
                break;
        }
    }
    long totalWait = 0;
    void** wait = (void *) malloc(sizeof(void**));
    signal(SIGSEGV, signalHandler);
    if ((lists = malloc(sizeof(SortedList_t) * nlists)) == NULL) {
        fprintf(stderr, "Error with malloc lists: %s\n", strerror(errno));
        exit(1);
    }
    createSublistsAndLocks();
    nElements = nthreads * iterations;
    elements = malloc(nElements * sizeof(SortedListElement_t));
    if(elements == NULL){
        fprintf(stderr, "Error with elements memory allocations: %s\n", strerror(errno));
        exit(2);
    }
    setKeys();
    pthread_t* threads = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
    struct timespec s, e;
    int stat = clock_gettime(CLOCK_MONOTONIC, &s);
    if(stat < 0){
        fprintf(stderr, "Error with clock_gettime start: %s\n", strerror(errno));
        exit(1);
    }
    int i;
    for(i = 0; i < nthreads; i++){
        if (pthread_create((threads + i), NULL, threadMain, (void*)(elements + iterations * i)) != 0){
            fprintf(stderr, "Error with pthread_create: %s\n", strerror(errno));
            exit(1);
        }
    }
    i = 0;
    for (; i < nthreads; i++)
    {
        if (pthread_join(threads[i], wait) != 0){
            fprintf(stderr, "Error with pthread_join: %s\n", strerror(errno));
            exit(1);
        }
        totalWait += (long)* wait;      
    }
    stat = clock_gettime(CLOCK_MONOTONIC, &e);
    if(stat < 0){
        fprintf(stderr, "Error with clock_gettime end: %s\n", strerror(errno));
        exit(1);
    }
    //print csv
    i = 0;
    int l = 0;
    for(;i < nlists; i++){
        l = SortedList_length(lists + i);
        if(l != 0){
            fprintf(stderr, "Error with completely clearing list: %s\n", strerror(errno));
            exit(2);
        }
    }
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
    
    long numOps = nthreads * iterations * 3;
    long lockWait = totalWait/numOps;
    long diff = e.tv_sec - s.tv_sec;
    long diffn = e.tv_nsec - s.tv_nsec;
    long runTime = (1000000000 * diff) + diffn;
    long avgTime = runTime/numOps;
    long nmlists = nlists;
    printLine(nthreads, iterations, nmlists, numOps, runTime, avgTime, lockWait);
    
    if (syncOp == 's') {
 	    free (spin);
 	}
    else if (syncOp == 'm') {
        for (i = 0; i < nlists; i++)
    	  	pthread_mutex_destroy(mutex+ i);
    	free(mutex);
 	}
    free(threads);
    free(elements);
	free(lists);
    free(wait);
    exit(0);
}