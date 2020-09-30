//NAME: William Chen
//EMAIL: billy.lj.chen@gmail.com
//ID: 405131881

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include "SortedList.h"

int opt_yield = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    if (!list || !element)
        return;
    SortedListElement_t *ptr = list->next;
    if (opt_yield & INSERT_YIELD){
        sched_yield();
    }
    while (ptr != list && strcmp(ptr->key, element->key) <= 0)
		ptr = ptr->next;
    element->next = ptr;
    element->prev = ptr->prev;
	ptr->prev->next = element;
	ptr->prev = element;
}

int SortedList_delete( SortedListElement_t *element){
    if(element == NULL){
        return 1;
    }
    if(element->next->prev != element){
        return 1;
    }
    if (element->prev->next != element) {
        return 1;
    }
    if (opt_yield & DELETE_YIELD)
		sched_yield();
    element->next->prev = element->prev;
    element->prev->next = element->next;
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if (list == NULL) {
        return NULL;
    }
    SortedListElement_t* ptr = list->next;
    while(ptr != list){
        if (opt_yield & LOOKUP_YIELD)
		    sched_yield();
        if (strcmp(key, ptr->key) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list){
    int length = 0;
    SortedListElement_t* ptr = list->next;
    while(ptr != list){
        if (ptr->next->prev != ptr){
			return -1;
        } 
        if (ptr->prev->next != ptr){
            return -1;
        }
        if (opt_yield & LOOKUP_YIELD)
		    sched_yield();
        length++;
        ptr = ptr->next;
    }
    return length;
}

