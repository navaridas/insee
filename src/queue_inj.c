/**
* @file
* @brief	Tools for the injection queues used in FSIN routers.

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
* Initializes an injection queue.
*
* Making it empty.
* 
* @param q The injection queue to be initialized.
*/
void inj_init_queue (inj_queue *q) {
	q->head = q->tail = 0;
}

/**
* Calculates the length of an injection queue.
* 
* @param q An injection queue.
* @return The number of phits in the injection queue.
*/
long inj_queue_len (inj_queue *q) {
	long aux;

	aux = q->tail - q->head;
	if (aux < 0) aux += inj_ql;
	return aux;
}

/**
* Calculates the free space in an injection queue.
* 
* @param q An injection queue.
* @return the number of free phits in the injection queue.
*/
long inj_queue_space (inj_queue *q) {
	long aux;

	aux = q->tail - q->head;
	if (aux < 0) aux += inj_ql;
	aux = (inj_ql-1)-aux;
	return aux;
}

/**
* Inserts a phit in an injection queue.
*
* Requires a buffer with room for the phit. Otherwise, panics
* 
* @param q An injection queue.
* @param i The phit to insert.
*/
void inj_ins_queue (inj_queue *q, phit *i) {
	if (inj_queue_len(q) == (inj_ql-1)) 
		panic("Inserting a phit in a full injection queue");
	else {
		q->tail = (q->tail + 1)%inj_ql;    
		(q->pos)[q->tail] = *i;
	}
}

/**
* Inserts some clones of a phit in an injection queue.
* 
* Requires enough space. Otherwise, panics.
* 
* @param q An injection queue.
* @param i The phit to be inserted.
* @param copies Number of copies of i.
*/
void inj_ins_mult_queue (inj_queue *q, phit *i, long copies) {
	long aux;

	for (aux = 0; aux < copies; aux++) {
		if (inj_queue_len(q) == (inj_ql-1)) 
		panic("Inserting multiple phits in a full injection queue");
		else {
			q->tail = (q->tail + 1)%inj_ql;    
			(q->pos)[q->tail] = *i;
		}
	}
}

/**
* Take the first phit in an injection queue.
* 
* Removes the head phit from the injection queue & returns it.
* Requires a non-empty queue. Otherwise, panics.
* 
* @param q An injection queue.
* @param i The removed phit is returned here.
*/
void inj_rem_queue (inj_queue *q, phit *i) {
	if (inj_queue_len(q) == 0) 
		panic("Removing the head of an empty injection queue");
	else {
		q->head = (q->head + 1)%inj_ql;
		*i = (q->pos)[q->head];
	}
}

