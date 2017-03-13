/**
* @file
* @brief	Event management functions used in trace-driven simulation.
*
* Event: A task that must be done (not occurred yet).
* Occurred: A task that has been done (total or partially completed).
*
*@author Javier Navaridas

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez, J. Navaridas

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <string.h>

#include "globals.h"
#include "misc.h"
#include "event.h"

#if (TRACE_SUPPORT != 0)

/**
* Initializes an event queue.
*
* @param q a pointer to the queue to be initialized.
*/
void init_event (event_q *q) {
	q->head = NULL;
	q->tail = NULL;
}

/**
* Adds an event to a queue.
*
* @param q a pointer to a queue.
* @param i the event to be added to q.
*/
void ins_event (event_q *q, event i) {
	event_n *e;
	e=malloc(sizeof(event_n));
	e->ev=i;
	e->next = NULL;

	if(q->head==NULL){ // Empty Queue
		q->head = e;
		q->tail = e;
	}
	else{
		q->tail->next = e;
		q->tail = e;
	}
}

/**
* Uses the first event in the queue.
*
* Takes an event an increases its packet count.
* When it reaches the length of the event, this is erased from the queue.
*
* @param q A pointer to a queue.
* @param i A pointer to the event to do.
*/
void do_event (event_q *q, event *i) {
	event_n *e;
	if (q->head==NULL)
		panic("Using event from an empty queue");
	e = q->head;
	e->ev.count++;
	*i = e->ev;
	if (i->count == i->length){
		q->head=q->head->next;
		free (e);
		if (q->head==NULL)
			q->tail=NULL;
	}
}


/**
* Uses the first event in the queue.
*
* Takes an event an increases its packet count.
* When it reaches the length of the event, this is erased from the queue.
*
* @param q A pointer to a queue.
* @param i A pointer to the event to do.
* @param i A pointer to the event to do.
*/
void do_event_n_times (event_q *q, event *i, CLOCK_TYPE increment) {
	event_n *e;
	if (q->head==NULL)
		panic("Using event from an empty queue");
	e = q->head;
	e->ev.count+=increment;
	*i = e->ev;
	if (i->count == i->length){
		q->head=q->head->next;
		free (e);
		if (q->head==NULL)
			q->tail=NULL;
	}
	if (i->count > i->length){
		panic("Increment in do_event_n_times exceeded the count");
	}
}

/**
* Looks at the first event in a queue.
*
* @param q A pointer to the queue.
* @return The first event in the queue (without using nor modifying it).
*/
event head_event (event_q *q) {
	if (q->head==NULL)
		panic("Getting event from an empty queue");
	return q->head->ev;
}

/**
* Deletes the first event in a queue.
*
* @param q A pointer to the queue.
*/
void rem_head_event (event_q *q) {
	event_n *e;
	if (q->head==NULL)
		panic("Deleting event from an empty queue");
	e = q->head;
	q->head=q->head->next;
	free (e);
	if (q->head==NULL) q->tail=NULL;
}

/**
* Is a queue empty?.
*
* @param q A pointer to the queue.
* @return TRUE if the queue is empty FALSE in other case.
*/
bool_t event_empty (event_q *q){
	return (q->head==NULL);
}

#if (TRACE_SUPPORT > 1)

/**
* Initializes all ocurred events lists.
*
* There is a list in each router for each posible source of messages.
* Increases memory usage but reduces look-up time for trace driven simulation. For large-scale systems, this can be a limiting factor.
*
* @param l A pointer to the list to be initialized.
*/
void init_occur (event_l **l){
	int i=0;
	for (i=0; i<nprocs; i++)
		(*l)[i].first=NULL;
}

/**
* Inserts an event's occurrence in an event list.
*
* If the event is in the list, then its count is increased. Otherwise a new event is created
* in the occurred event list.
*
* @param l A pointer to a list.
* @param i The event to be added.
*/
void ins_occur (event_l **l, event i){
	event_n *e = (*l)[i.pid].first;
	event_n *aux = NULL;

	if (e==NULL) {	// List is Empty
		// Create a new occurred event
		aux=malloc(sizeof(event_n));
		i.count = 1;
		aux->ev = i;
		aux->next = (*l)[i.pid].first;
		(*l)[i.pid].first = aux;
		return;
	}

	if (e->ev.type == i.type && e->ev.pid == i.pid &&
		e->ev.task == i.task && e->ev.length == i.length &&
		e->ev.count < e->ev.length)	{
		// The occurence is the first element.
		e->ev.count++;
		return;
	}

	while (e->next!=NULL) {
		if (e->next->ev.type == i.type && e->next->ev.pid == i.pid &&
			e->next->ev.task == i.task && e->next->ev.length == i.length &&
			e->next->ev.count < e->next->ev.length) {
			// Another element in the list
			e->next->ev.count++;
			return;
		}
		e = e->next;
	}

	// It is not in the list, so we create a new occurred event
	aux = malloc(sizeof(event_n));
	i.count = 1;
	aux->ev = i;
	aux->next = (*l)[i.pid].first;
	(*l)[i.pid].first = aux;
}

/**
* Has an event completely occurred?.
*
* If it has totally occurred, this is, the event is in the list and its count is equal to
* its length, then it is deleted from the list.
*
* @param l a pointer to a list.
* @param i the event we are seeking for.
* @return TRUE if the event has been occurred, elseway FALSE
*/
bool_t occurred (event_l **l, event i){
	event_n *e = (*l)[i.pid].first;
	event_n *aux;

	if (e==NULL)	// List is Empty
		return B_FALSE;
	if (e->ev.type == i.type && e->ev.pid == i.pid && e->ev.count == e->ev.length &&
		e->ev.task == i.task && e->ev.length == i.length) {
		aux = e->next;
		free(e);
		(*l)[i.pid].first = aux;
		return B_TRUE;
	}

	while (e->next!=NULL) {
		if (e->next->ev.type == i.type && e->next->ev.pid == i.pid &&
			e->next->ev.task == i.task && e->next->ev.length == i.length &&
			e->next->ev.count == e->next->ev.length) {
			// Delete from list
			aux = e->next->next;
			free(e->next);
			e->next = aux;
			return B_TRUE;
		}
		e = e->next;
	}
	return B_FALSE;
}
#endif // Trace support with multilist #occurs

#if (TRACE_SUPPORT == 1)

/**
* Initializes all ocurred events lists.
*
* There is only one list in each router.
*
* @param l A pointer to the list to be initialized.
*/
void init_occur (event_l *l){
	(*l).first=NULL;
}

/**
* Inserts an event's occurrence in an event list.
*
* If the event is in the list, then its count is increased. Otherwise a new event is created
* in the occurred event list.
*
* @param l A pointer to a list.
* @param i The event to be added.
*/
void ins_occur (event_l *l, event i){
	event_n *e = (*l).first;
	event_n *aux = NULL;

	if (e==NULL) {	// List is Empty
		// Create a new occurred event
		aux=malloc(sizeof(event_n));
		i.count = 1;
		aux->ev = i;
		aux->next = (*l).first;
		(*l).first = aux;
		return;
	}

	if (e->ev.type == i.type && e->ev.pid == i.pid &&
		e->ev.task == i.task && e->ev.length == i.length &&
		e->ev.count < e->ev.length)	{
		// The occurence is the first element.
		e->ev.count++;
		return;
	}

	while (e->next!=NULL) {
		if (e->next->ev.type == i.type && e->next->ev.pid == i.pid &&
			e->next->ev.task == i.task && e->next->ev.length == i.length &&
			e->next->ev.count < e->next->ev.length) {
			// Another element in the list
			e->next->ev.count++;
			return;
		}
		e = e->next;
	}

	// There is not in the list, so we create a new occurred event
	aux = malloc(sizeof(event_n));
	i.count = 1;
	aux->ev = i;
	aux->next = (*l).first;
	(*l).first = aux;
}

/**
* Has an event completely occurred?.
*
* If it has totally occurred, this is, the event is in the list and its count is equal to
* its length, then it is deleted from the list.
*
* @param l a pointer to a list.
* @param i the event we are seeking for.
* @return TRUE if the event has been occurred, elseway FALSE
*/
bool_t occurred (event_l *l, event i){
	event_n *e = (*l).first;
	event_n *aux;

	if (e==NULL)	// List is Empty
		return B_FALSE;
	if (e->ev.type == i.type && e->ev.pid == i.pid && e->ev.count == e->ev.length &&
		e->ev.task == i.task && e->ev.length == i.length) {
		aux = e->next;
		free(e);
		(*l).first = aux;
		return B_TRUE;
	}

	while (e->next!=NULL) {
		if (e->next->ev.type == i.type && e->next->ev.pid == i.pid &&
			e->next->ev.task == i.task && e->next->ev.length == i.length &&
			e->next->ev.count == e->next->ev.length) {
			// Delete from list
			aux = e->next->next;
			free(e->next);
			e->next = aux;
			return B_TRUE;
		}
		e = e->next;
	}
	return B_FALSE;
}
#endif // Trace support with single list #occurs

#endif

