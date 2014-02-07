/** 
* @file
* @brief	Implementation of a double linked dinamic list.
* Used by Execution driven simulation module.
*/

#include <stdlib.h>
#include <stdio.h>
#include "list.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/**
* Create a element to be inserted in the list.
*
* @param dataPointer A pointer to the element data.
* @return The element.
*/
listElement *CreateListElement( void * dataPointer ){
	listElement *firstListElement;

	firstListElement = (listElement *) malloc( sizeof(listElement) * 1 );
	firstListElement->previous = NULL;
	firstListElement->next = NULL;
	firstListElement->pointer = dataPointer;

	return firstListElement;
}

/**
* Destroy an element.
* 
* @param thisElement The element to destroy.
*/
void DestroyListElement( listElement *thisElement ){
	listElement *ll;
	ll = thisElement->next;
	if( ll )
		ll->previous = thisElement->previous;
	ll = thisElement->previous;
	if( ll )
		ll->next = thisElement->next;
	free( thisElement );
}

/**
* Adds an element before another one.
* 
* @param thisElement The element to insert before.
* @param dataPointer The data of the element to insert.
* @return The inserted element.
*/
listElement *AddBeforeElement( listElement *thisElement, void *dataPointer ){
	listElement *newListElement;
	listElement *tmp;

	if( thisElement->pointer) { 
		newListElement = (listElement *) malloc( sizeof(listElement) * 1 );
		tmp = thisElement->previous;
		newListElement->next = thisElement;
		newListElement->previous = tmp;
		newListElement->pointer = dataPointer;
		thisElement->previous = newListElement;
		if( tmp )
			tmp->next = newListElement;
	}
	else {
		/* if this list is empty, insert the pointer*/
		thisElement->pointer = dataPointer;
		newListElement = thisElement;
	}
	return newListElement;
}

/**
* Adds an element after another one.
* 
* @param thisElement The element to insert after.
* @param dataPointer The data of the element to insert.
* @return The inserted element.
*/
listElement *AddAfterElement( listElement *thisElement, void *dataPointer ){
	listElement *newListElement;
	listElement *tmp;
 
	if( thisElement->pointer) { 
		newListElement = (listElement *) malloc( sizeof(listElement) * 1 );
		newListElement->previous = thisElement;
		newListElement->next = thisElement->next;
		newListElement->pointer = dataPointer;
		tmp = thisElement->next;
		thisElement->next = newListElement;
		if( tmp )
			tmp->previous = newListElement;
	}
	else {
		/* if this list is empty, insert the pointer*/
		thisElement->pointer = dataPointer;
		newListElement = thisElement;
	}
	return newListElement;
}

/**
* Creates an empty list.
* 
* @return The empty list.
*/
list *CreateVoidList(){
	list  *newList;

	newList = (list *) malloc( sizeof( list ) * 1 );
	newList->emptyList = TRUE;
	newList->first = NULL;
	newList->last = NULL;
	newList->nextInLoop = NULL;
	return newList;
}

/**
* Destroy a list.
* 
* @param theList The lñist to destroy.
*/
void  DestroyList( list **theList ){
	listElement *ll1;
	listElement *ll2;
	list *thisList = *theList;
	if( ! thisList ) return;

	if( !thisList->emptyList ) {
		ll1 = thisList->first;
		while( ll1 ) {
			ll2 = ll1;
			ll1 = ll1->next;
			free( ll2 );
		}
	}
	free( thisList );

	*theList = NULL;
}

/**
* Add an element at the begining of a list.
* 
* @param thisList The list to insert into.
* @param pointer The data of the element to insert.
*/ 
void  AddFirst( list *thisList, void *pointer){
	if( ! thisList )
		printf("Adding element to NULL pointer\n");
	else if( thisList->emptyList ) {
		thisList->first = CreateListElement( pointer);
		thisList->last = thisList->first;
		thisList->emptyList = FALSE;
	}
	else
		thisList->first		= AddBeforeElement( thisList->first, pointer);
}

/**
* Add an element at the end of a list.
* 
* @param thisList The list to insert into.
* @param pointer The data of the element to insert.
*/ 
void  AddLast ( list *thisList, void *pointer){
	if( ! thisList )
		printf("Adding element to NULL pointer\n");
	else if( thisList->emptyList ) {
		thisList->first		= CreateListElement( pointer);
		thisList->last		= thisList->first;
		thisList->emptyList	= FALSE;
	}
	else
		thisList->last		= AddAfterElement( thisList->last, pointer); 
}

/**
* Start a loop in a list
* 
* @param thisList The list to loop into.
* @return A pointer to the loop.
*/ 
void *StartLoop( list *thisList ){
	if( ! thisList ) return NULL;
	if( thisList->emptyList ) return NULL;
	thisList->nextInLoop = thisList->first->next;
	return thisList->first->pointer;
}

/**
* Get an element of a list.
* 
* @param thisList The list to get the next element.
* @return The element's data.
*/ 
void *GetNext ( list *thisList ){
	void *dataPointer;

	if( thisList->nextInLoop == NULL )
		dataPointer = NULL;
	else {
		dataPointer = thisList->nextInLoop->pointer;
		thisList->nextInLoop = thisList->nextInLoop->next;
	}
	return dataPointer;
}

/**
* Assert if data is in the list.
* 
* @param thisList The list to look in.
* @param dataPointer The data to look for.
* @return TRUE/FALSE.
*/ 
int   IsInList( list *thisList, void *dataPointer ){
	listElement *ll;

	if( ! thisList )
		return FALSE;

	if( !thisList->emptyList ) {
		ll = thisList->first;
		while( ll ) {
			if( dataPointer == ll->pointer) return TRUE;
			ll = ll->next;
		}
	}
	return FALSE;
}

/**
* Assert if list is empty.
* 
* @param thisList The list to look in.
* @return TRUE/FALSE.
*/ 
int   IsEmpty( list *thisList ){
	return thisList->emptyList;
}

/**
* Number of elements in the list.
* 
* @param thisList The list to look in.
* @return The number of elements in the list.
*/ 
int   ElementsInList( list *thisList ){
	listElement *ll;
	int num;

	if( ! thisList )
		return 0;

	num = 0;

	ll = thisList->first;
	if( ! ll ) return 0;
	while( ll ) {
		num++;
		ll = ll->next;
	}
	return num;
}

/**
* Remove some data from a list.
* 
* @param thisList The list to look in.
* @param dataPointer The data to remove.
*/ 
void  RemoveFromList( list *thisList, void *dataPointer ){
	listElement *ll;
 
	if( ! thisList ) return;

	if( !thisList->emptyList ) {
		ll = thisList->first;
		while( ll ) {
			if( dataPointer == ll->pointer ) {
				if( thisList->last == thisList->first ) {
					thisList->emptyList = TRUE;
					ll->pointer = NULL;
					/* Anadido por RIDRUEJO */
					DestroyListElement( ll );
					thisList->first = thisList->last = NULL;
					/* Fin de anadido por RIDRUEJO */
				}
				else {
					if( ll == thisList->first )
						thisList->first = ll->next;
					else if( ll == thisList->last )
						thisList->last = ll->previous;
					DestroyListElement( ll );
				}
				return;
			}
			ll = ll->next;
		}
	}
}

/**
* Print the list.
* 
* @param thisList The list to print.
*/ 
void PrintList( list *thisList ){
	listElement *ll;
	if( ! thisList )
		printf("NULL list \n");
	else if( !thisList->emptyList ) {
		ll = thisList->first;
		while( ll ) {
			printf(" %d %d %d \n",*(int *)ll->previous,*(int *)ll->pointer,*(int *)ll->next );
			ll = ll->next;
		}
	}
	printf("===============\n");
}
