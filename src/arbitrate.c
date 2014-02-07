/**
* @file
* @brief	Declaration of FSIN functions to perform arbitration.
*
* FSIN Functional Simulator of Interconnection Networks
* Copyright (2003-2011) J. Miguel-Alonso, A. Gonzalez, J. Navaridas
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "globals.h"

/**
* An array of input ports that are requesting the output port under arbitration.
*/
static bool_t * candidates;

/**
* Used when in_transit_priority is ON. It limits how often an injection port is assigned to an output port.
* When using timeout-based congestion detection ipr_l[0] is the value of ipr when the router is not congested and
* ipr_l[1] is the value of ipr to be used when the router is congested. Other running modes only use ipr_l[0].
*/
static long ipr_l[2];

/**
* Last port to check when arbitrating the consumption port.
*/
port_type last_port_arb_con;

/**
* Initialization of the structures needed to perform arbitration.
*/
void arbitrate_init(void) {
	candidates = alloc(sizeof(bool_t) * n_ports);
	ipr_l[1] = (long) (intransit_pr * RAND_MAX);

	if (timeout_upper_limit>0)
		ipr_l[0] = 0;
	else
		ipr_l[0] = ipr_l[1];

	if (topo<DIRECT) // Direct Topologies.
	    last_port_arb_con=p_inj_first;
	else // Trees and hybred topologies.
	    last_port_arb_con=nchan;
}

/**
* Tries to reserve an output port.
*
* An output port has selected the input port (s_p) that will be connected to an output port (d_p) at node i.
* @param i The node in which the reservation is performed.
* @param d_p The reserved destination port.
* @param s_p The source port which is reserving.
*/
void reserve(long i, port_type d_p, port_type s_p) {
	port_type p;
	network[i].p[s_p].aop = d_p;				// Annotation at input port
	network[i].p[s_p].bet = B_TRIAL_0;		// Success reserving!! Reset my next bet
	network[i].p[d_p].sip = s_p;				// Annotation of source input port
	network[i].p[d_p].ri = s_p;				// Annotation of last used input
}

/**
* Performs consumption of a single phit.
*
* For "single" consumption, the consumption port (just one) is arbitrated just like any other one.
* @param i The node in which the consumption is performed.
*/
void arbitrate_cons_single(long i) {
	arbitrate(i, p_con);
	return;
}

/**
* Performs consumption of all phits that have arrived to a node.
*
* For "multiple" consumption, all input ports can access to the consumption port.
* @param i The node in which the consumption is performed.
*/
void arbitrate_cons_multiple(long i) {
	port_type s_p;
	
	for (s_p=0; s_p<last_port_arb_con; s_p++) {
		if (network[i].p[p_con].req[s_p]) {
			// Has input port "s_p" requested consumption port?
			if (!queue_len(&network[i].p[s_p].q))
			{
				printf("node %d, p_con %d, s_p %d\n",i,p_con,s_p);
				panic("Trying to assign consumption port to empty input queue - multiple");
			}
			network[i].p[s_p].aop = p_con;
			network[i].p[s_p].bet = B_TRIAL_0; // Success reserving!! Reset my next bet -- Only for adaptive
			continue;
		}
	}
}

/**
* Main arbitration function for direct topologies. Implements the virtual function arbitrate.
*
* An output port can be assigned to an input port in a range [first, last)
* If IPR allows it, all input ports (transit + injection) are in the range [0, p_con).
* If not, we check the range of transit ports [0, p_inj_first) and, later,
* if not assignment has been done, we check the range of injection ports
* [p_inj_first, <p_con). NOTE that the last port in the range is NOT included
* in the arbitration. Actual selection is done by arbitrate_select(),
* which make take different values: arbitrate_select_fifo(), etc.
*
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
*
* @see arbitrate
* @see arbitrate_select
* @see arbitrate_select_fifo
* @see arbitrate_select_longest
* @see arbitrate_select_round_robin
* @see arbitrate_select_random
* @see arbitrate_select_age
*/
void arbitrate_direct(long i, port_type d_p) {
	port_type s_p, firstlimit, lastlimit;

	if (network[i].p[d_p].sip != P_NULL)
		return;  // Cannot arbitrate twice
		
	firstlimit = 0;

	if (ipr_l[network[i].congested] && (rand() <= ipr_l[network[i].congested]))
		lastlimit = p_inj_first;	// If priority is ON for in_transit traffic, injection ports
									// are not included in the arbitration process
	else
        lastlimit = p_con;

	s_p = arbitrate_select(i, d_p, firstlimit, lastlimit);
	// If arbitration did not succeed, maybe an injection port can be assigned...
	if ((s_p == NULL_PORT) && (lastlimit != p_con))
		s_p = arbitrate_select(i, d_p, p_inj_first, p_con);
	if (s_p != NULL_PORT)
		reserve(i, d_p, s_p);
}

/**
* Main arbitration function for the indirect cube . Implements the virtual function arbitrate.
*
* This functions will only check those ports that make sense:
* the NIC when arbitrating a consumption port,
* the injection ports when arbitrating a NIC,
* all the transit ports when arbitrating a switch.
* NOTE that the transit ports can be affected by the Intransit Priority Restriction.
* In that case, ports that are connected to the NICS are checked in a second round.
*
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
*
* @see arbitrate
* @see arbitrate_select
* @see arbitrate_select_fifo
* @see arbitrate_select_longest
* @see arbitrate_select_round_robin
* @see arbitrate_select_random
* @see arbitrate_select_age
*/
void arbitrate_icube(long i, port_type d_p) {
	port_type s_p, firstlimit, lastlimit;

	if (network[i].p[d_p].sip != P_NULL)
		return;  // Cannot arbitrate twice

	if (i<nprocs)   // In a leaf node
	{
		if (d_p==p_con)	// only the NIC will require using the consumption port.
		{
            firstlimit = 0;
			lastlimit = nchan;
		}
		else if (d_p<nchan)   // the NIC will be required only by the injection ports.
		{
			firstlimit = p_inj_first;
			lastlimit = p_con;
		}
		else return; //Should not be checking this
	}
	else    // switches do not have injection ports.
		if (ipr_l[network[i].congested] && (rand() <= ipr_l[network[i].congested])) // Checking IPR...
		{
			firstlimit=nodes_per_switch*nchan;
			lastlimit=p_inj_first;
		}
		else
		{
            firstlimit=0;
			lastlimit=p_inj_first;
		}
	// Congestion with timeouts.
	s_p = arbitrate_select(i, d_p, firstlimit, lastlimit);
	// If arbitration did not succeed, maybe an injection port can be assigned...
	if ((s_p == NULL_PORT) && (firstlimit != 0) && (i>=nprocs))
		s_p = arbitrate_select(i, d_p, 0, nodes_per_switch*nchan);
	if (s_p != NULL_PORT)
	{
		reserve(i, d_p, s_p);
	}
}

/**
* Main arbitration function for the trees. Implements the virtual function arbitrate.
*
* This functions will only check those ports that make sense:
* the NIC when arbitrating a consumption port,
* the injection ports when arbitrating a NIC,
* all the transit ports when arbitrating a switch.
*
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
*
* @see arbitrate
* @see arbitrate_select
* @see arbitrate_select_fifo
* @see arbitrate_select_longest
* @see arbitrate_select_round_robin
* @see arbitrate_select_random
* @see arbitrate_select_age
*/
void arbitrate_trees (long i, port_type d_p) {
	port_type s_p, firstlimit, lastlimit;

	if (network[i].p[d_p].sip != P_NULL)
		return;  // Cannot arbitrate twice

	if (i<nprocs)   // In a leaf node.
	{
		if (d_p==p_con)	// only the NIC will require using the consumption port.
		{
            firstlimit = 0;
			lastlimit = nchan;
		}
		else    // the NIC will be required only by the injection ports.
		{
			firstlimit = p_inj_first;
			lastlimit = p_con;
		}
	}
	else    // switches do not have injection ports.
	{
        firstlimit = 0;
		lastlimit = p_inj_first;
	}

	s_p = arbitrate_select(i, d_p, firstlimit, lastlimit);
	if (s_p != NULL_PORT)
		reserve(i, d_p, s_p);
}

/**
* Select the port that requested the port first of all the given ports.
*
* Given a range of input-injection ports, select the one that requested the output port first
* Time of request is stored in network[i].p[d_p].req[s_p].
*
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
* @param first The first port for looking to.
* @param last The next port from the last to looking to. This port is not included.
* @return The selected port, or NULL_PORT if there isnt anyone.
*
* @see arbitrate
* @see arbitrate_select
*/
port_type arbitrate_select_fifo(long i, port_type d_p, port_type first, port_type last) {
	port_type s_p, selected_port;
	CLOCK_TYPE time_of_selected, min;

	time_of_selected = CLOCK_MAX;

	for (s_p=first; s_p<last; s_p++) {
		min = network[i].p[d_p].req[s_p];
		if (min!=0) {
			if (min < time_of_selected) {
				time_of_selected = min;
				selected_port = s_p;
			}
		}
	}
	
	if (time_of_selected != CLOCK_MAX) 
		return (selected_port);
	else 
		return (NULL_PORT);
}

/**
* Select the port with the biggest queue length.
*
* Given a range of input-injection ports, select the one whose queue occupation is longer.
* We take a look to the queues in a round-robin fashion, starting with the last lucky
* input port (network[i].p[d_p].ri), if several ports have the same occupation, the first 
* visited will be the selected one. 
*
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
* @param first The first port for looking to.
* @param last The next port from the last to looking to. This port is not included.
* @return The selected port, or NULL_PORT if there isnt anyone.
*
* @see arbitrate
* @see arbitrate_select
*/
port_type arbitrate_select_longest(long i, port_type d_p, port_type first, port_type last) {
	port_type s_p, selected_port, visited;
	long len_of_selected, pl;
	long dif=last-first;

	s_p = first + ((network[i].p[d_p].ri + 1) % dif);
	if (s_p >= last) s_p = first;
	len_of_selected = -1;

	for (visited=first; visited<last; visited++) {
		if (network[i].p[d_p].req[s_p]) {
			pl = queue_len(&network[i].p[s_p].q);
			if (pl > len_of_selected) {
				len_of_selected = pl;
				selected_port = s_p;
			}
		}
		s_p = first + ((s_p + 1) % (dif));
		if (s_p >= last) s_p = first;
	}
	if (len_of_selected != -1) return(selected_port);
	else return(NULL_PORT);
}

/**
* Select the next occupied port using round-robin.
*
* Given a range of input-injection ports, select one in a round-robin fashion. The last used is stored
* in network[i].p[d_p].ri
*
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
* @param first The first port for looking to.
* @param last The next port from the last to looking to. This port is not included.
* @return The selected port, or NULL_PORT if there isnt anyone.
*
* @see arbitrate
* @see arbitrate_select
*/
port_type arbitrate_select_round_robin(long i, port_type d_p, port_type first, port_type last) {
	port_type s_p, visited;
	long dif=last - first;
	s_p = first + ((network[i].p[d_p].ri + 1) % dif);
	if (s_p >= last) s_p = first;
	for (visited=first; visited<last; visited++) {
		if (network[i].p[d_p].req[s_p])
			return(s_p);
		s_p = first + ((s_p + 1) % dif);
		if (s_p >= last) s_p = first;
	}
	return(NULL_PORT);
}

/**
* Random selection of a port.
*
* Given a range of input-injection ports, select one of them randomly.
* 
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
* @param first The first port for looking to.
* @param last The next port from the last to looking to. This port is not included.
* @return The selected port, or NULL_PORT if there isnt anyone.
* @see arbitrate
*/
port_type arbitrate_select_random(long i, port_type d_p, port_type first, port_type last) {
	port_type s_p;
	long rp, ncand;

	// First we calculate the number of input ports requesting this output
	ncand = 0;
	for(s_p=first; s_p<last; s_p++) {
		if (!network[i].p[d_p].req[s_p])
			candidates[s_p] = FALSE; // s_p is not requesting
		else {
			candidates[s_p] = TRUE;
			ncand++;
		}
	}
	// Now throw the dice and select the lucky one
	rp = ztm(ncand);
 	for (s_p=first; s_p<last; s_p++) {
		if (!candidates[s_p]) continue;
		if (rp-- == 0)
			return(s_p);
	}
	return(NULL_PORT);
}

/**
* Select the port containing the first injected packet.
*
* Given a range of input-injection ports, select the one with the oldest packet,
* measured since it was injected.
* 
* @param i The node in which the arbitration is performed.
* @param d_p The destination port for wich the arbitration is performed.
* @param first The first port for looking to.
* @param last The next port from the last to looking to. This port is not included.
* @return The selected port, or NULL_PORT if there isnt anyone.
* @see arbitrate
*/
port_type arbitrate_select_age(long i, port_type d_p, port_type first, port_type last) {
	port_type s_p, selected_port;
	CLOCK_TYPE time_of_selected, min;
	phit *p;

	time_of_selected = CLOCK_MAX;

	for (s_p=first; s_p<last; s_p++) {
		if (!network[i].p[d_p].req[s_p])
			continue;
		p = head_queue(&network[i].p[s_p].q);
		min = pkt_space[p->packet].inj_time;
		if (min < time_of_selected) {
			time_of_selected = min;
			selected_port = s_p;
		}
	}
	if (time_of_selected != CLOCK_MAX) 
		return (selected_port);
	else 
		return (NULL_PORT);
}
