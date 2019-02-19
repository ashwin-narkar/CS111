/*
NAME: Ashwin Narkar
ID: 204585474
EMAIL: ashwin.narkar@gmail.com
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "SortedList.h"

int opt_yield;

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	//first element is next of head pointer which is NULL

	SortedList_t *currElement = list->next;
	SortedList_t *head = list;


	if (opt_yield & INSERT_YIELD) {
		sched_yield();
	}
	//critical section begins here

	while (currElement != head) { //circularly linked so if we go back to head we messed up kinda
		if (strcmp(currElement->key,element->key) >= 0) {	//we have arrived if currElement is bigger than the one we wanna add
			//add element before currElement

			break;
		}
		currElement = currElement->next;
	}
	element->prev = currElement->prev;
	element->next = currElement;
	currElement->prev->next = element;
	currElement->prev = element;
	
}



/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete(SortedListElement_t *element) {
	SortedList_t *currElement = element;
		
	if (currElement->prev->next != currElement || currElement->next->prev != currElement) {
		return -1;
	}


	if (opt_yield & DELETE_YIELD) {
		sched_yield();
	}

	//Critical section here
	currElement->prev->next = currElement->next;
	currElement->next->prev = currElement->prev;
	free(currElement);
	return 0;
}



/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	SortedList_t *currElement = list->next;
	SortedList_t *head = list;
	//fprintf(stderr, "%s\n", currElement->key);


	if (opt_yield & LOOKUP_YIELD) {
		sched_yield();
	}
	//Critical section here

	while (currElement != head) {
		
		if (strcmp(currElement->key,key) == 0) {
			//fprintf(stderr, "found\n");
			return currElement;
		}
		currElement = currElement->next;
	}
	
	return NULL;
}


/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list) {
	int length = 0;
	SortedList_t *currElement = list->next;
	while (currElement != list) {
		if (currElement->next->prev != currElement || currElement->prev->next != currElement) {
			return -1;
		}
		currElement = currElement->next;
		length++;
	}
	return length;
}