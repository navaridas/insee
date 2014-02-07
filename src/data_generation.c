/**
* @file
* @brief	Data generation module of FSIN.

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

#include "globals.h"
#include "packet.h"
#include "pattern.h"

#include <stdlib.h>

#define POP_SIZE 1
//#define POP_SIZE 262144

long pop[POP_SIZE];	///< The population when using population based distributions. Should be implemented dinamically only when needed.
long *next_dest;	///< A list with all the destinations for a node (used in distribution patterns).

void read_population();
void read_histogram();

/**
* The maximum capacity of the network(in packets).
*
* The aggregated capacity of network's transit queues.
*/
static long net_capacity;

/**
* The maximun queue occupancy allowed in global congestion control.
*
* When this value is exceeded all data generation is stopped.
*/
static long congestion_limit;

/**
* An array for packet count in shotmode simulation.
*
* It stores the number of packets injected by each node, so its size is equal to the network size.
*/
static long *count=NULL;

/**
* Select the shortest injection queue.
*
* Selects the port with SHORTEST injection + buffer utilization.
* 
* @param i The node in which the injection is performed.
* @param dest The destination node. Actually not used in this function.
* @return The port in which inject.
*
* @see select_input_port
*/
port_type select_input_port_shortest(long i, long dest) {
	port_type e, currport, selport;
	long minlen, currlen ;
	inj_queue *ib;
	queue *iq;

	minlen = RAND_MAX;
	currport = rand() % ninj;
	selport = currport;
	for (e=0; e<ninj; e++) {
		ib = &(network[i].qi[currport]); // ib is a pointer to inj buffer
		iq = &(network[i].p[currport+p_inj_first].q); // iq is a pointer to inj queue
		currlen = inj_queue_len(ib) + queue_len(iq);
		if (currlen < minlen) {
			minlen = currlen;
			selport = currport;
		}
		currport = (currport + 1)%ninj;
	}
	return selport;
}

/**
* Select an injection queue using strict DOR prerouting.
*
* When using DIMENSION_ORDER_ROUTING most packets are injectec in X+ or X-
* 
* @param i The node in which the injection is performed.
* @param dest The destination node.
* @return The port in which inject.
*
* @see select_input_port
*/
port_type select_input_port_dor_only(long i, long dest) {
	routing_r r;
	dim j;
	port_type p=NULL_PORT;

	r = calc_rr(i, dest);

	switch (routing) {
		case DIMENSION_ORDER_ROUTING:
			for (j=D_X; j<ndim; j++)
				if (r.rr[j] > 0) p=nways*j;
				else if (r.rr[j] < 0) p=nways*j+1;
				break;
		case DIRECTION_ORDER_ROUTING:
			for (j=D_X; j<ndim; j++)
				if (r.rr[j] > 0) p=nways*j;
			for (j=D_X; j<ndim; j++)
				if (r.rr[j] < 0) p=nways*j+1;
			break;
		default:;
	}
	free(r.rr);
	if (p==NULL_PORT)
		panic("Bad pre-routing");
	else
		return p;
}

/**
* Select an injection queue using the packet's routing record.
*
* Taking into consideration all the possible profitable dim/way options, it injects
* in that with shortest injection+buffer utilization
* 
* @param i The node in which the injection is performed.
* @param dest The destination node.
* @return The port in which inject.
*
* @see select_input_port
*/
port_type select_input_port_shortest_profitable(long i, long dest) {
	port_type currport, selport;
	long minlen, currlen ;
	inj_queue *ib;
	queue *iq;
	routing_r r;
	dim j;

	r = calc_rr(i, dest);

	minlen = RAND_MAX;
	currport = rand() % ninj;
	selport = currport;

	for (j=D_X; j<ndim; j++) {
		if (r.rr[j]) {
			if (r.rr[j] > 0) currport = nways*j;
			else if (r.rr[j] < 0) currport = nways*j+1;
			ib = &(network[i].qi[currport]); // ib is a pointer to inj buffer
			iq = &(network[i].p[currport+p_inj_first].q); // iq is a pointer to inj queue
			currlen = inj_queue_len(ib) + queue_len(iq);
			if (currlen < minlen) {
				minlen = currlen;
				selport = currport;
			}
		}
	}
	free (r.rr);
	return selport;
}

/**
* Select an injection queue using DOR prerouting and if not possible in the shortest port.
* 
* It uses select_input_port_dor but, if the corresponding injector is full, it tries to inject 
* in the port with shortest injection+buffer utilization.
* 
* @param i The node in which the injection is performed.
* @param dest The destination node.
* @return The port in which inject.
*
* @see select_input_port
*/
port_type select_input_port_dor_shortest(long i, long dest) {
	port_type selport;
	inj_queue *ib;

	selport = select_input_port_dor_only(i, dest);
	ib = &(network[i].qi[selport]); // ib is a pointer to inj buffer
	if (inj_queue_space(ib) >= pkt_len) return selport;
	selport = select_input_port_shortest(i, dest);
	return selport;
}

/**
* Select an injection queue using the packet's routing record.
* 
* Select the injector for the direction in which the packet has the highest number of hops.
* 
* @param i The node in which the injection is performed.
* @param dest The destination node.
* @return The port in which inject.
*
* @see select_input_port
*/
port_type select_input_port_lpath(long i, long dest) {
	port_type currport, selport;
	long maxlen, currlen ;
	routing_r r;
	dim j;

	r = calc_rr(i, dest);
	selport = 0;
	maxlen = -1;
	for (j=D_X; j<ndim; j++) {
		if (r.rr[j]) {
			if (r.rr[j] > 0)
				currport = nways*j;
			else if (r.rr[j] < 0)
				currport = nways*j+1;
			currlen = labs(r.rr[j]);
			if (currlen > maxlen) {
				maxlen = currlen;
				selport = currport;
			}
		}
	}
	free(r.rr);
	return selport;
}

/**
* Generate all the phits of a packet and put them in the corresponding injection port.
*
* @param packet The packet to inject.
* @param iport The port in where inject to.
*/
void generate_phits(unsigned long packet, port_type iport) {
	dim j;
	phit p;
	inj_queue *qi;
	long node=pkt_space[packet].from;

	qi = &(network[node].qi[iport]);
	if (inj_queue_space(qi) < pkt_space[packet].size)
		panic("Should not be injecting phits");

	p.packet = packet;
	p.pclass = RR;

	if(plevel & 16) {
		printf("T: "PRINT_CLOCK" - N: %4ld Packet(id %5ld) Injected (%ld->%ld)\n",sim_clock, node, packet, node, pkt_space[packet].to);
		if (topo<DIRECT){
			printf("                  rr: [");
			for (j=D_X; j<ndim; j++)
				printf("%ld ", pkt_space[packet].rr.rr[j]);
			printf("]\n");
		}
	}

	if (pkt_space[packet].size == 1) {
		p.pclass = RR_TAIL;
		inj_ins_queue(qi, &p);
	} else {
		inj_ins_queue(qi, &p); // The routing record
		p.pclass = INFO;
		inj_ins_mult_queue(qi, &p, pkt_space[packet].size-2); // The body
		p.pclass = TAIL;
		inj_ins_queue(qi, &p);
	}

	if (plevel & 1)
		sources[pkt_space[packet].from][pkt_space[packet].to]++;

	sent_count++;
#if (BIMODAL_SUPPORT != 0)
	msg_sent_count[pkt_space[packet].mtype]++;
#endif /* BIMODAL */
	sent_phit_count += pkt_space[packet].size;
}

/**
* Generate a packet to inject.
*
* If there is no packet saved then generates a packet using the defined pattern.
* Then try to inject the generated packet & if it cannot be injected and packet
* dropped is not allowed, save the packet and try to use it later.
* 
* @see generate_phits
* 
* @param i The node in which the injection is performed.
*/
void generate_pkt(long i) {
	long d, n;
	unsigned long pkt;
	double aux;
	inj_queue *qi;
	port_type iport;
	packet_t packet;

//	if (network[i].source==NO_SOURCE) // Should not be testing this -- paranoid mode.
//	{
//		printf("node %d\n",i);
//		panic("Trying to generate a packet in a NO_SOURCE node");
//	}

	if (!drop_packets && network[i].pending_packet > 0){
		packet = network[i].saved_packet;
	}
	else{
		if (network[i].triggered==0){
			if (network[i].source==INDEPENDENT_SOURCE)
			{
				aux=rand();
				if (aux > aload )
					return;
#if (BIMODAL_SUPPORT != 0)
				else{
					// Long or Short messages are generated.
					if ( aux < lm_load )
						packet.mtype = LONG_MSG;
					else
						packet.mtype = SHORT_MSG;
				}
#endif /* BIMODAL */
			}
		}
		else
			network[i].triggered--;
	
		switch (pattern) {
		// RANDOM DESTINATIONS
		case HOTREGION:
			aux = ((double)rand()/(double)(RAND_MAX));
			if (aux <= 0.25)
				do {
					d = (long)(0.125*nprocs*rand()/(RAND_MAX+1.0));
				} while (d == i);
			else
				do {
					d = (long)(1.0*nprocs*rand()/(RAND_MAX+1.0));
				} while (d == i);
			break;
		case HOTSPOT:
			do {
				aux = ((double)rand()/(double)(RAND_MAX));
                        	if (aux <= 0.02)
                                        d = 0; //((nodes_x/2)*(rand()%2))+(nodes_x*(nodes_y/2)*(rand()%2));	// The hot spots are (0,0); (0,Y/2); (X/2, Y/2); (X/2, 0);
                        	else
                                        d = (long)(1.0*nprocs*rand()/(RAND_MAX+1.0));
                        } while (d == i);
			break;
		case LOCAL:
			do {
				long	dst[3]={0,0,0},	// the coordinates of the destination.
						n,		// the dimension
						r;		// a random number.
				double	rnd;	// the same number in range [0..1)

				for (n=0; n<ndim; n++){
					r=rand();
					rnd=(1.0*r)/(RAND_MAX+1.0);
					if (rnd<0.5)
					{
						dst[n]=1-(r%3);	// [-1, 1]
						//printf("1(%f). %d, ",rnd,dst[n]);
					}
					else if (rnd<0.75)
					{
						dst[n]=r%4;		// [-3,-2] U [ 2, 3]
						if (dst[n]<2)
							dst[n]-=3;
						//printf("2(%f). %d, ",rnd,dst[n]);
					}
					else if (rnd<0.875)
					{
						dst[n]=r%8;		// [-7,-4] U [ 4, 7]
						if (dst[n]<4)
							dst[n]-=7;
						//printf("3(%f). %d, ",rnd,dst[n]);
					}
					else
					{
						if (n==D_X)		// rest of the network.
							dst[D_X]=(r%(nodes_x-15))+8;
						if (n==D_Y)
							dst[D_Y]=(r%(nodes_y-15))+8;
						if (n==D_Z)
							dst[D_Z]=(r%(nodes_z-15))+8;
						//printf("4(%f). %d, ",rnd,dst[n]);
					}
				}
				dst[D_X]=mod(dst[D_X]+network[i].rcoord[D_X],nodes_x);
				dst[D_Y]=mod(dst[D_Y]+network[i].rcoord[D_Y],nodes_y);
				dst[D_Z]=mod(dst[D_Z]+network[i].rcoord[D_Z],nodes_z);

				d=dst[D_X]+(dst[D_Y]*nodes_x)+(dst[D_Z]*nodes_x*nodes_y);
			} while (d == i);
			break;
		case UNIFORM:
			do {
				d = (long)(1.0*nprocs*rand()/(RAND_MAX+1.0));
			} while (d == i);
			break;
		case SEMI:
			if ( i%nodes_x < nodes_x/2 )
				do {
					long x,y;
					x= rand()%(nodes_x/2);
					y= rand()%nodes_y;
					d = x+(nodes_x*y);
				} while (d == i);
			else
				return;
			break;

		// DISTRIBUTIONS
		case DISTRIBUTE:
		case RSDIST:
			d = next_dest[i];
			if (d == i)
				d = (d+1)%nprocs;
			next_dest[i] = (d+1)%nprocs;
			break;

		// NOW, THE PERMUTATIONS
		case TRANSPOSE:
		case TORNADO:
		case COMPLEMENT:
		case BUTTERFLY:
		case SHUFFLE:
		case REVERSAL:
			d = next_dest[i];
			if (i == d){
				return ;
			}
			break;
		case POPULATION:
			if (topo<DIRECT)	// for direct topologies, ttorus should have a separate one.
			{
				long dst[3]={0,0,0},	// the number of hops in each dimension.
				     r;			// the total number of hops
				do{
					r=pop[rand()%POP_SIZE];
				}while (r==0 || r>(nodes_x+nodes_y+nodes_z)/2);

				if (ndim==3){
					dst[D_Z]=rand()%(1+r);
					if (dst[D_Z]>nodes_z/2)
						dst[D_Z]=nodes_z/2;
					r-=dst[D_Z];
					if (rand() & 1)
						dst[D_Z]=-dst[D_Z];
				}
				if (ndim>1){
					dst[D_Y]=rand()%(1+r);
					if (dst[D_Y]>nodes_y/2)
                                                dst[D_Y]=nodes_y/2;
                                        r-=dst[D_Y];
                                        if (rand() & 1)
                                                dst[D_Y]=-dst[D_Y];
                                }

				dst[D_X]=r;
				if (dst[D_X]>nodes_x/2)
					dst[D_X]=nodes_x/2;
                                r-=dst[D_X];
                                if (rand() & 1)
                                        dst[D_X]=-dst[D_X];

				dst[D_X]=mod(dst[D_X]+network[i].rcoord[D_X],nodes_x);
                                dst[D_Y]=mod(dst[D_Y]+network[i].rcoord[D_Y],nodes_y);
                                dst[D_Z]=mod(dst[D_Z]+network[i].rcoord[D_Z],nodes_z);
                                d=dst[D_X]+(dst[D_Y]*nodes_x)+(dst[D_Z]*nodes_x*nodes_y);
			}
			// Trees and icubes should have their own traffic generator.
			break;
		case HISTOGRAM:
			break;

#if (TRACE_SUPPORT != 0)
		// Trace Based traffic
		case TRACE:
			if (network[i].source==INDEPENDENT_SOURCE) { // Background traffic - uniform
				do {
					d = (long)(1.0*nprocs*rand()/(RAND_MAX+1.0));
				} while (d == i || network[d].source!=INDEPENDENT_SOURCE);
			} else {
				if (!event_empty(&network[i].events)){
					event e;
					while (!event_empty(&network[i].events) && head_event(&network[i].events).type==RECEPTION){
						e = head_event(&network[i].events);
						if (occurred(&network[i].occurs, e))
							rem_head_event(&network[i].events);
						else
							break;
					}
					if (!event_empty(&network[i].events) && head_event(&network[i].events).type==SENDING){
						do_event(&network[i].events, &e);
						packet.task = e.task;
						packet.length = e.length;
						d=e.pid;
					}
					else
						return ;
				}
				else
					return ;
			}
			break;
#endif
		default:
			panic("Bad traffic pattern");
		}
		packet.to = d;
		packet.from = i;
		packet.size = pkt_len;
	}// End of generating new (not-saved) packet

	iport = select_input_port(packet.from, packet.to);
	qi = &(network[packet.from].qi[iport]);

#if (BIMODAL_SUPPORT != 0)
	if ( (n = network[i].pending_packet) == 0 )
		if (packet.mtype == LONG_MSG)
			n=msglength;
		else
			n=1;
#else
	if ( (n = network[i].pending_packet) == 0 )
		n=1;
#endif /* BIMODAL */

	for (;n>0;n--){
		if (inj_queue_space(qi) < packet.size){
			if (!drop_packets){
				network[i].saved_packet = packet;
				network[i].pending_packet = n;
			}
			else{
				dropped_count++;
				dropped_phit_count += packet.size;
			}
			return ;
		}
		else{
			packet.tt = sim_clock;
			packet.rr = calc_rr(packet.from, packet.to);

			if (plevel&4)
				inj_dst[packet.rr.size]++;

			packet.inj_time = sim_clock;  // Some additional info
			packet.n_hops = 0;
			inj_phit_count += pkt_len;
		}
#if (BIMODAL_SUPPORT != 0)
		if (packet.mtype == LONG_MSG && n == 1)
			packet.mtype = LONG_LAST_MSG;
#endif /* BIMODAL */

		pkt=get_pkt();
		pkt_space[pkt] = packet;
		generate_phits(pkt, iport);
		packet.size = pkt_len;
#if (PCOUNT!=0)
		network[i].pcount+=packet.size;
#endif
		if(shotmode)
			count[i]--;
	}
	network[i].pending_packet=0;
}

/**
* Generates data when allowed.
*
* Injection may be stopped by the global congestion control, by the parameter mpackets
* in fsin.conf, or by the shot mode when a burst is finished.
* 
* @param i The node in which the data must be generated.
*/
void data_generation(long i) {
	if (global_q_u > congestion_limit)
		return;
	if (i>=nprocs)
		panic("Generating packets in a undefined node");
#if (TRACE_SUPPORT != 0)
	{
		event e;
		if(!event_empty(&network[i].events) && head_event(&network[i].events).type==COMPUTATION){
			e = head_event(&network[i].events);
			do_event(&network[i].events, &e);
		}
	}
#endif
	generate_pkt(i);
}

/**
* Performs the data generation when running in shotmode.
* 
* @param reset When TRUE, starts a new shot.
*/
void datagen_oneshot(bool_t reset) {
	long i;

	switch (reset) {
		case TRUE: // new shot.
			if (!count)
				count = alloc(sizeof (long)*nprocs);
			for (i=0; i<nprocs; i++)
				count[i] = shotsize;
				// One cycle is wasted here.
			return;
		case FALSE: // inject
			for (i=0; i<nprocs; i++) {
				if (count[i])
					data_generation(i);
			}
			return;
		default:
			panic("Should not be here in datagen_oneshot");
			return;
	}
}

/**
* Moves data from an injection buffer to an injection queue.
*
* Performs data movement from an injection buffer, which is used for data transmision between
* the router and its node processor(s), to the injection port which takes part in the router logic.
* 
* @param i The node in which the injection is performed.
*/
void data_injection(long i) {
	phit ph;
	inj_queue *ib;
	queue *iq;
	port_type e, iport;

	if (parallel_injection) {
		for (e=0; e<ninj; e++) {
			ib = &(network[i].qi[e]); // ib is a pointer to inj buffer
			iq = &(network[i].p[e+p_inj_first].q); // iq is a pointer to inj queue
			while (queue_space(iq) && inj_queue_len(ib)) {
				inj_rem_queue(ib, &ph);
				ins_queue(iq, &ph);
			}
		}
	}
	else {
		iport = network[i].injecting_port;
		if (iport != NULL_PORT) {
			ib = &(network[i].qi[iport]); // ib is a pointer to selected inj buffer
			iq = &(network[i].p[iport+p_inj_first].q); // iq is a pointer to selected inj queue
			if (queue_space(iq) && inj_queue_len(ib)) {
				inj_rem_queue(ib, &ph);
				ins_queue(iq, &ph);
				if (ph.pclass >= TAIL) {
					network[i].injecting_port = NULL_PORT;
					network[i].next_port = (iport + 1) % ninj;
				}
			}
		}
		else { // Port to inject not selected, arbitration is required
			for (e=0; e<ninj; e++) {
				iport = (network[i].next_port+e)%ninj;
				ib = &(network[i].qi[iport]);
				if (!inj_queue_len(ib)) continue;
				iq = &(network[i].p[iport+p_inj_first].q);
				if (queue_space(iq)) {
					network[i].injecting_port = iport; // Grab it!!
					inj_rem_queue(ib, &ph);
					ins_queue(iq, &ph);
					return;
				}
			}
		}
	}
}

/**
* Initializes Injection.
*
* Calculates all needed variables for injection & prepares all structures.
*/
void init_injection	(void) {
	long i, scount;
	long d;

	if (topo<DIRECT)
		net_capacity = NUMNODES * (radix * nways * nchan) * buffer_cap;
	else
		net_capacity = (NUMNODES - nprocs) * (radix * nways * nchan) * buffer_cap;

	congestion_limit = (long)(net_capacity*(global_cc /	100.0));

	next_dest = alloc(sizeof(long)*nprocs);
	scount = 0;
	for (i=0; i<nprocs; i++) {
		network[i].source=INDEPENDENT_SOURCE;
		d = -1;
		switch (pattern) {
			case DISTRIBUTE:
				d = (i+1)%nprocs;
				break;
			case RSDIST:
				do {
					d = ztm(nprocs);
				} while (i == d);
				break;
			case TRANSPOSE:
				d = transpose(i, nprocs);
				break;
			case TORNADO:
				// B. Towles, W. J. Dally. "Worst-case Traffic for Oblivious Routing Functions." Volume 1, Feb. 2002.
				d = (i + (nodes_x-1)/2)%nprocs;
				break;
			case COMPLEMENT:
				d = complement(i, nprocs);
				break;
			case BUTTERFLY:
				d = butterfly(i, nprocs);
				break;
			case SHUFFLE:
				d = shuffle(i, nprocs);
				break;
			case REVERSAL:
				d = reversal(i, nprocs);
				break;
			case UNIFORM:
			case HOTREGION:
			case SEMI:
			case HOTSPOT:
				break;
			case LOCAL:
				// this is done for each node, but it is not necessary, may be moved to other place
				break;
			case POPULATION:
				if (i==nprocs-1)
					read_population();
				break;
			case HISTOGRAM:
				if (i==nprocs-1)
					read_histogram();
				break;
#if (TRACE_SUPPORT != 0)
			case TRACE:
				if (i==nprocs-1)
					read_trace();
				break;
#endif
			default:
				panic("Should not be here initializing patterns");
				break;
		}
		next_dest[i] = d;
		if (d != i) scount++;
	}
	if (shotmode)
		total_shot_size = scount*shotsize;
}

/**
 * Reads the population from a file. EXPERIMENTAL
 * 
 * A population is a collection of distances at which the injected packets will be sent.
 * Allows feeding the simulation with user-defined distance distributions.
 */
void read_population(){
	FILE *fpop;
	char buffer[64];
	long i;

	if((fpop = fopen(trcfile, "r")) == NULL){
                printf("%s\n",trcfile);
                panic("Population file not found in current directory");
	}

	for (i=0; i<POP_SIZE; i++){
		fgets(buffer,64,fpop);
		pop[i]=atol(buffer);
	}

	fclose(fpop);
}

/**
 * Reads the histogram from a file. EXPERIMENTAL
 * 
 * An histogram with the distance distribution that the simulation should follow. 
 * Allows feeding the simulation with user-defined distance distributions.
 */
void read_histogram(){
}
