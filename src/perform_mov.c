/**
* @file
* @brief	Definition of traffic movement & consumption functions.

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

static void phit_moved(long i, long n_n, port_type s_p, port_type d_p, phit ph);
static void drop_transit(long i);

/**
 * Drops in-transit phits/packets
 * 
 * @param i the node where the phit to drop is.
 */
void drop_transit(long i){
	port_type s_p;
	phit ph;

#if (TRACE_SUPPORT != 0)
	if (pattern==TRACE)
	    panic("Should not be dropping packet when trace-driven simulation");
#endif
#if (EXECUTION_DRIVEN != 0)
	panic("Should not be dropping packet when execution-driven simulation");
#endif
	for (s_p=0; s_p<p_con; s_p++) {
		if (network[i].p[s_p].aop == p_drop) {
			rem_queue(&(network[i].p[s_p].q), &ph);	// Drop
			dropped_phit_count++;
#if (PCOUNT!=0)
			network[i].pcount--;
#endif
			if(plevel & 32)
				printf("T: %"PRINT_CLOCK" - N: %4ld Phit class %1d dropped\n", sim_clock, i, ph.pclass);
			if(plevel & 16 && (ph.pclass == RR || ph.pclass == RR_TAIL))
				printf("T: %"PRINT_CLOCK" - N: %4ld Packet(id %5ld) header dropped %"PRINT_CLOCK" c. after inj.\n",
						sim_clock, i, ph.packet, sim_clock - pkt_space[ph.packet].inj_time );

			if (ph.pclass >= TAIL) { // TAIL or RR_TAIL
				network[i].p[s_p].aop = P_NULL; // Free reservation
				network[i].p[p_con].sip = P_NULL;
				network[i].p[s_p].tor = CLOCK_MAX;
				transit_dropped_count++;
				free_pkt(ph.packet);
				if (plevel & 16)
					printf("T: %"PRINT_CLOCK" - N: %4ld Packet(id %5ld) dropped\n", sim_clock, i,ph.packet);
			}
		}
	}
}

/**
* Consume a single phit.
*
* This "Single" version is used when there is a single consumption port,
* shared among all the VCs. Arbitration is required.
* 
* @param i The number of the node in which the consumption is performed.
*/
void consume_single(long i) {
	port_type s_p;	// Index of source port.
	phit ph;		// Phit to moved.
	queue *q;		// The queue where the phit is stored.

	s_p = network[i].p[p_con].sip;
	if (s_p == P_NULL)
		return;		// Nobody has this port assigned
	if (network[i].p[s_p].aop != p_con)
		panic("Bad assignment - consume single");
	q = &(network[i].p[s_p].q);		// Transit queue to get phit from
	rem_queue(q, &ph);
	phit_away(i, s_p, ph);
#if (PCOUNT!=0)
	network[i].pcount--;
#endif
}

/**
* Consume one/many phits.
*
* This is the "multiple" version, meaning that in a cycle it is possible to
* consume phits from all VCs.
* 
* @param i The number of the node in which the consumption is performed.
*/
void consume_multiple(long i) {
	port_type s_p;
	phit ph;

	for (s_p=0; s_p<p_inj_first; s_p++) {
		if (network[i].p[s_p].aop == p_con) {
			rem_queue(&(network[i].p[s_p].q), &ph);	// Consume NOW
			if (i>=nprocs)
				printf ("WARNING packet consumed in communication element %d [%d -> %d] %d!!!\n",i,pkt_space[ph.packet].to, pkt_space[ph.packet].from, pkt_space[ph.packet].n_hops );
			phit_away(i, s_p, ph);
#if (PCOUNT!=0)
			network[i].pcount--;
#endif
		}
	}
}

/**
* Performs the movement of the data in a direct topology.
* 
* @param inject If TRUE new data generation is performed.
*
* @see init_functions
* @see data_movement
*/
void data_movement_direct(bool_t inject) {
	long i,	// Node id
		 e,	// port number
		 ee;// port requested by port 'e'
	dim j; way k;

	for (i=0; i<NUMNODES; i++) {
		if (plevel & 8)
			stats(i);
		if (inject)
			data_generation(i);
		data_injection(i);

#if (PCOUNT!=0)
		if (network[i].pcount){
#endif
			for (e=0; e<=p_con; e++)
				for (ee=0; ee<p_con; ee++)
					network[i].p[e].req[ee] = (CLOCK_TYPE) 0L;
			for (e=0; e<p_con; e++)
				request_port(i, e);
			arbitrate_cons(i);
			for (e=0; e<p_con; e++)
				arbitrate(i, e);

			// Congestion with timeouts.
			if (timeout_upper_limit>0) {
				network[i].timeout_counter++;
				if (network[i].timeout_counter > timeout_upper_limit){
					network[i].congested = (network[i].timeout_packet != NULL_PORT);
					network[i].timeout_counter = (CLOCK_TYPE) 0L;
					network[i].timeout_packet = NULL_PACKET;
				}
			}
#if (PCOUNT!=0)
		}
#endif
	}

	for (i=0; i<NUMNODES; i++) {
#if (PCOUNT!=0)
		if (network[i].pcount){
#endif
			consume(i);
			for (j=D_X; j<ndim; j++)
				for (k=UP; k<nways; k++)
					advance(i, dir(j, k));
		}
#if (PCOUNT!=0)
	}
#endif
}

/**
* Performs the movement of the data in an indirect topology.
* 
* @param inject If TRUE new data generation is performed.
*
* @see init_functions
* @see data_movement
*/
void data_movement_indirect(bool_t inject) {
	long i,		// Node id
		 e,		// port number
		 ee;	// port requested by port 'e'
	long to;
	dim j; way k;

	for (i=0; i<NUMNODES; i++) {
		if (plevel & 8)
			stats(i);
		if (i<nprocs){	// This is a NIC. There are only ports for injection/consumption and 1 output port.
			if (inject)
				data_generation(i);
			data_injection(i);

#if (PCOUNT!=0)
			if (network[i].pcount){
#endif
				for (e=0; e<nchan; e++)	// only injection can ask for the output port.
					for (ee=p_inj_first; ee<p_con; ee++)
						network[i].p[e].req[ee] = (CLOCK_TYPE) 0L;
				for (ee=0; ee<nchan; ee++)	// Only the output port can ask for the consumption port.
					network[i].p[p_con].req[ee] = (CLOCK_TYPE) 0L;

				for (e=0; e<nchan; e++)	// output port requesting
					request_port(i, e);
				for (e=p_inj_first; e<p_con; e++)	// injection port requesting
					request_port(i, e);

				arbitrate_cons(i);
				for (e=0; e<nchan; e++)	// output port arbitration
					arbitrate(i, e);
				for (e=p_inj_first; e<p_con; e++)	// injection port arbitration
					arbitrate(i, e);
#if (PCOUNT!=0)
			}
#endif
		}
		else{
#if (PCOUNT!=0)
			if (network[i].pcount){
#endif
				for (e=0; e<=p_con; e++)
					for (ee=0; ee<p_con; ee++)
						network[i].p[e].req[ee] = (CLOCK_TYPE) 0L;
				for (e=0; e<=p_inj_last; e++)
					request_port(i, e);

				arbitrate_cons(i);
				for (e=0; e<p_inj_last; e++)
					arbitrate(i, e);
#if (PCOUNT!=0)
			}
#endif
		}

		// Congestion with timeouts.
		if (timeout_upper_limit>0) {
			network[i].timeout_counter++;
			if (network[i].timeout_counter > timeout_upper_limit){
				network[i].congested = (network[i].timeout_packet != NULL_PORT);
				network[i].timeout_counter = (CLOCK_TYPE) 0L;
				network[i].timeout_packet = NULL_PACKET;
			}
		}
	}

	to=1;
	for (i=0; i<NUMNODES; i++) {
		consume(i);
		if (i==nprocs)	// This way, in CPU nodes only advance the NIC port.
			to=radix;	// From nprocs on, to=radix.
		for (j=0; j<to; j++)
			advance(i, j);
	}
}
/**
* Advance packets.
*
* Move a phit from an output port to the corresponding input port in the neighbour
* 
* @param n The number of the node.
* @param p The physichal port id to advance.
*/
void advance(long n, long p) {
	long n_n;	// Id of neighbor node
	channel l;	// Index for virtual/escape channels
	long visited;
	port_type s_p, d_p, d_np;	// source / destination port ids
	queue *q;
	phit ph;

	if ((n_n=network[n].nbor[p]) == NULL_PORT)
		return;
	l = network[n].op_i[p];

	for (visited=0; visited<nchan; visited++) {
		d_p = port_address(p, l);
		s_p = network[n].p[d_p].sip;
		if (s_p != P_NULL) {
			if (network[n].p[s_p].aop != d_p)
			{
				printf("node %d, port %d, d_p %d, s_p %d\n", n,p, d_p, s_p);
				panic("Bad assignment - move port");
			}
			q = &(network[n].p[s_p].q);     // Transit queue to get phit from
			network[n].op_i[p] = l;			// For next phit
			if (!queue_len(q))
			{
				printf("node %d, port %d, d_p %d, s_p %d\n", n,p, d_p, s_p);
				panic("Should have something to move");
			}
			rem_queue(q, &ph);
			d_np= port_address(network[n].nborp[p],l);

			phit_moved(n, n_n, s_p, d_np, ph);
#if (PCOUNT!=0)
			network[n].pcount--;
			network[n_n].pcount++;
#endif

			if (ph.pclass >= TAIL) {
				network[n].op_i[p] = (l+1)%nchan;	// Next time assign to another virtual channel
				network[n].p[s_p].aop = P_NULL;		// Free reservations
				network[n].p[s_p].tor = CLOCK_MAX;
				network[n].p[d_p].sip = P_NULL;
			}
			return;
		}
		l = (l+1)%nchan;
	}
}

/**
* A phit has arrived to destination & is consumed.
*
* Gets all kind of statistics, and print the corresponding traces.
* "old" version, simpler: does not interact with TrGen, just collects statistics.
* 
* @param i The node in which the consumption is performed.
* @param s_p The input port in wich the phit is.
* @param ph The arrived phit.
*/
void phit_away(long i, port_type s_p, phit ph) {
	CLOCK_TYPE del;

	rcvd_phit_count++;
	if (i!=pkt_space[ph.packet].to){
		printf("packet %d, from %d to %d arrives to %d\n",ph.packet, pkt_space[ph.packet].from, pkt_space[ph.packet].to, i);
		panic("Wrong destination");
	}
	if(plevel & 32)
		printf("T: %"PRINT_CLOCK" - N: %4ld Phit class %1d consumed\n", sim_clock, i, ph.pclass);

	if(ph.pclass == RR || ph.pclass == RR_TAIL){
		if(plevel & 4)
			con_dst[pkt_space[ph.packet].n_hops]++;
		if(plevel & 16)
			printf("T: %"PRINT_CLOCK" - N: %4ld Packet(id %5ld) header reaches node %"PRINT_CLOCK" c. after inj.\n",
					sim_clock, i, ph.packet, sim_clock - pkt_space[ph.packet].inj_time );
	}

	if (ph.pclass >= TAIL) { // TAIL or RR_TAIL
		network[i].p[s_p].aop = P_NULL; // Free reservation
		network[i].p[p_con].sip = P_NULL;
		network[i].p[s_p].tor = CLOCK_MAX;
		del = sim_clock - pkt_space[ph.packet].inj_time;
		acum_delay += del;
		acum_sq_delay += del*del;
		if (rand()<= trigger)
			network[i].triggered += trigger_min + rand()%trigger_dif;

		if (del > max_delay)
			max_delay = del;
		rcvd_count++;

		acum_hops += pkt_space[ph.packet].n_hops;

		if (i == monitored)
			source_ports[s_p]++;

		if (plevel & 1)
			destinations[pkt_space[ph.packet].from][pkt_space[ph.packet].to]++;

#if (BIMODAL_SUPPORT != 0)
		msg_acum_delay[pkt_space[ph.packet].mtype] += del;
		msg_acum_sq_delay[pkt_space[ph.packet].mtype] += del*del;
		if (del > msg_max_delay[pkt_space[ph.packet].mtype])
			msg_max_delay[pkt_space[ph.packet].mtype]= del;
		msg_rcvd_count[pkt_space[ph.packet].mtype]++;
#endif /* BIMODAL */

#if (TRACE_SUPPORT != 0)
		if (pattern==TRACE){// Adds Event in an ocurred event's list
			event e;
			e.type=RECEPTION;
			e.pid=pkt_space[ph.packet].from;
			e.task=pkt_space[ph.packet].task;
			e.length=pkt_space[ph.packet].length;
			ins_occur(&network[i].occurs, e);
		}
#endif

#if (EXECUTION_DRIVEN != 0)
		SIMICS_phit_away(i, ph);
#endif

		free_pkt(ph.packet);
		if(plevel & 16)
			printf("T: %"PRINT_CLOCK" - N: %4ld Packet(id %5ld) consumed\n", sim_clock, i,ph.packet);
	}
}

/**
* Moves a phit from a router to one of its neighbors.
*
* Complement of advance, actually moves a phit from one output port to an input port
* 
* @param i The number of the source node.
* @param n_n The neighbor node(node to move to).
* @param s_p Source port (input port of the node).
* @param d_p Destination port (input port of the neighbor).
* @param ph The phit to move.
*/
void phit_moved(long i, long n_n, port_type s_p, port_type d_p, phit ph) {
	queue *n_q;
	dim j; way k;
	n_q = &(network[n_n].p[d_p].q);

	if (queue_space(n_q)<1)
	{
		printf("(%d) %d.%d -> %d.%d \n",ph.packet,i,s_p,n_n, d_p);
		panic("Should not be moving when no space in receiving port");
	}

	if ((ph.pclass == RR) || (ph.pclass == RR_TAIL)) {
		// Congestion with timeouts.
		if (timeout_upper_limit > 0){
			if (network[n_n].timeout_packet == NULL_PORT) {
				network[n_n].timeout_counter = (CLOCK_TYPE) 0L;
				network[n_n].timeout_packet = ph.packet;
			}
			if (network[i].timeout_packet == ph.packet)	{
				if (network[i].timeout_counter < timeout_lower_limit)
					network[i].congested=FALSE;
				network[i].timeout_counter = (CLOCK_TYPE) 0L;
				network[i].timeout_packet = NULL_PACKET;
			}
		}
		// Update routing record only for direct topologies.
		if (topo<DIRECT){
			j = port_coord_dim[d_p];
			k = port_coord_way[d_p];

			if (k == UP)
				pkt_space[ph.packet].rr.rr[j]--;
			else
				pkt_space[ph.packet].rr.rr[j]++;
		}

		// Update routing record only for direct topologies.
		if (topo==ICUBE && n_n >= nprocs && i >= nprocs){
			j = port_coord_dim[d_p];	// Should be checking the destination port in the neighbor node.
			k = port_coord_way[d_p];	// Should be checking the destination port in the neighbor node.

			if (k == DOWN)	// In indirect cube d_p is the port opposite to the destination port in the neighbor node(d_np).
				if (pkt_space[ph.packet].rr.rr[j]<0)
					panic("going through - while rr is positive");
				else
				pkt_space[ph.packet].rr.rr[j]--;
			else
				if (pkt_space[ph.packet].rr.rr[j]>0)
					panic("going through + while rr is negative");
				else
				pkt_space[ph.packet].rr.rr[j]++;
		}

		if (s_p >= p_inj_first){
			CLOCK_TYPE del;
			injected_count++;

			del = sim_clock - pkt_space[ph.packet].inj_time;
			acum_inj_delay += del;
			acum_sq_inj_delay += del*del;
			if (del > max_inj_delay)
				max_inj_delay = del;
#if (BIMODAL_SUPPORT != 0)
			msg_injected_count[pkt_space[ph.packet].mtype]++;
			msg_acum_inj_delay[pkt_space[ph.packet].mtype] += del;
			msg_acum_sq_inj_delay[pkt_space[ph.packet].mtype] += del*del;
			if (del > msg_max_inj_delay[pkt_space[ph.packet].mtype])
				msg_max_inj_delay[pkt_space[ph.packet].mtype] = del;
#endif /* BIMODAL */
			if (i == monitored)
				dest_ports[d_p]++;
		}/* injection */
		pkt_space[ph.packet].n_hops++;
	}/* RR */

	ins_queue(n_q, &ph);

	if (plevel & 32)
		printf("T: %"PRINT_CLOCK" - N: %4ld Phit class %1d moved to %4ld via %ld\n", sim_clock, i, ph.pclass, n_n, d_p);

	if (plevel & 16 && (ph.pclass == RR || ph.pclass == RR_TAIL)) {
		printf("T: %"PRINT_CLOCK" - N: %4ld Packet(id %5ld) header departs towards %4ld\n", sim_clock, i, ph.packet, n_n);
		if (ph.pclass >= TAIL)
			printf("T: %"PRINT_CLOCK" - N: %4ld Packet(id %5ld) leaves node\n", sim_clock, i, ph.packet);
	}
	network[i].p[d_p].utilization++;
	if (i == monitored)
		port_utilization[d_p]++;
}
