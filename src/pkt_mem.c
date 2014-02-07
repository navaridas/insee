/**
* @file
* @brief	Definition of FSIN functions for packet management.
*
*@author Javier Navaridas

FSIN Functional Simulator of Interconnection Networks
Copyright (2003-2011) J. Miguel-Alonso, J. Navaridas

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
#include "packet.h"

/**
* Structure in wich all the packets are stored.
*
* It is created and allocated when stating simulation, and it must be not modify later.
* It is used in heap mode, so we try to use the last free packet to favor spatial locality.
* 
* @see pkt_init
*/
packet_t * pkt_space;

/**
* The maximum number of packets allowed.
*
* Normally is the number of nodes multiplied by the maximun capacity of each router.
*/
long pkt_max;

/**
* A list with ids of all the free packets.
*/
long * f_pkt;

/**
* The last position in use on the list.
*/
long last;

/**
* Initiates the memory allocation & the free packets structure.
*
* In pkt_space we allocate all the space needed for packet contain.
* This way our Memory needs are always the same and
* we must not alloc and free memory each time we use a packet.
*/
void pkt_init(){
	long i;
	pkt_max = ((NUMNODES * n_ports * buffer_cap)	// packets in network +
		+ (nprocs * ninj * binj_cap));				// packets in injectors.

	pkt_space=alloc(sizeof(packet_t)*pkt_max);
	f_pkt=alloc(sizeof(long)*pkt_max);

	for(i=0;i<pkt_max;i++)
		f_pkt[i]=i;
	last=pkt_max-1;
}

/**
* Frees a packet.
*
* Add the packet id. to the free packets
* 
* @param n The id of the packet to free.
*/
void free_pkt(unsigned long n){
	if (last==pkt_max)
		panic("Too much free packets");
	if (pkt_space[n].rr.rr!=NULL)
		free(pkt_space[n].rr.rr);
	f_pkt[++last]=n;
}

/**
* Get a free packet.
* 
* @return The id of the last used free packet.
*/
unsigned long get_pkt(){
	if (last<0)
		panic("Packet memory is FULL.\n       Something wrong is happening");
	return f_pkt[last--];
}
