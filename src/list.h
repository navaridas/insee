#ifndef LIST_H
#define LIST_H

#include "misc.h"

/**
* @file
* @brief	Definitions for the double linked dinamic list.
* Used by Execution driven simulation module.
*/

typedef struct _ListElement
{
	void	*previous; /* Null means first element */
	void	*next;     /* Null means last element */
	void	*pointer;
} listElement;
void         DestroyListElement( listElement * );
listElement *CreateListElement( void * );

listElement *AddBeforeElement( listElement *, void * );
listElement *AddAfterElement( listElement *, void * );

typedef struct _List
{
	bool_t emptyList;
	listElement	*first;
	listElement	*last;
	listElement	*nextInLoop;
} list;

void  DestroyList( list ** );
	// Before destroying the list,
	// probably, you should destroy the elements the list points to.
	// If they are not destroyed, they might be left "dangling"
	// if nobody else is pointing to them
list *CreateVoidList();

void  AddFirst( list *, void * );
void  AddLast ( list *, void * );
void *StartLoop( list * );
void *GetNext ( list * );
int   IsInList( list * , void * );
int   ElementsInList( list * );
void  RemoveFromList( list *, void *);
void  PrintList( list *thisList );

#endif /* LIST_H */

