/**
* @file
* @brief	Tools for the transit queues used in FSIN routers.

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez

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

#include "globals.h"

/**
* Initializes a queue.
*
* Making it empty.
*
* @param q The queue to initialize.
*/
void init_queue (queue *q) {
	q->head = q->tail = 0;
}

/**
* Calculates the length of a queue.
*
* @param q A queue.
* @return The number of phits in the queue.
*/
long queue_len (queue *q) {
	long aux;

	aux = q->tail - q->head;
	if (aux < 0)
		aux += tr_ql;
	return aux;
}

/**
* Calculates the free space in a queue.
*
* @param q A queue.
* @return the number of phits available in the queue.
*/
long queue_space (queue *q) {
	long aux;

	aux = q->tail - q->head;
	if (aux < 0)
		aux += tr_ql;
	aux = (tr_ql-1)-aux;
	return aux;
}

/**
* Looks at the first phit of a queue.
*
* Requires a non-empty queue. Otherwise, panics
*
* @param q A queue.
* @return A pointer to the first phit of the queue.
*/
phit * head_queue (queue *q) {
	long aux;

	if (queue_len(q) == 0){
		panic("Asking for the head of an empty queue");
		return NULL;
	}
	else {
		aux = (q->head + 1)%tr_ql;
		return &((q->pos)[aux]);
	}
}

/**
* Inserts a phit in a queue.
*
* Requires a buffer with room for the phit. Otherwise, panics
*
* @param q A queue.
* @param i The phit to be inserted.
*/
void ins_queue (queue *q, phit *i) {
	if (queue_len(q) == (tr_ql-1))
		panic("Inserting a phit in a full queue");
	else {
		q->tail = (q->tail + 1)%tr_ql;
		(q->pos)[q->tail] = *i;
	}
}

/**
* Inserts many (identical) copies of a phit "i" in queue "q"
*
* Requires enough space. Otherwise, panics.
*
* @param q A queue.
* @param i The phit to be cloned & inserted.
* @param copies Number of clones of i.
*/
void ins_mult_queue (queue *q, phit *i, long copies) {
	long aux;

	for (aux = 0; aux < copies; aux++)
		if (queue_len(q) == (tr_ql-1))
			panic("Inserting multiple phits in a full queue");
		else {
			q->tail = (q->tail + 1)%tr_ql;
			(q->pos)[q->tail] = *i;
		}
}

/**
* Take the first phit in a queue.
*
* Removes the head phit from queue & returns it via "i"
* Requires a non-empty queue. Otherwise, panics.
*
* @param q A queue.
* @param i The removed phit is returned here.
*/
void rem_queue (queue *q, phit *i) {
	if (queue_len(q) == 0)
		panic("Removing the head of an empty queue");
	else {
		q->head = (q->head + 1)%tr_ql;
		*i = (q->pos)[q->head];
	}
}

/**
* Removes the head of queue.
*
* Does not return anything. Requires a non-empty queue. Otherwise, panics.
*
* @param q A queue.
*/
void rem_head_queue (queue *q) {
	if (queue_len(q) == 0)
		panic("Removing the head of an empty queue");
	else
		q->head = (q->head + 1)%tr_ql;
}

