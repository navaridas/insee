/**
* @file
* @brief	Initialization of structures & functions required for simulation.

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

#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "misc.h"

/**
* Initialization of functions.
*
* Instances the virtual functions to the ones required for the simulation
*
* @see run_network
* @see neighbor
* @see calc_rr
* @see check_rr
* @see request_port
* @see consume
* @see arbitrate_cons
* @see arbitrate_select
* @see select_input_port
* @see data_movement
*/
void init_functions (void) {
	long i, j;

	if (shotmode) run_network = run_network_shotmode;
#if (TRACE_SUPPORT != 0)
	else if (pattern == TRACE) run_network = run_network_trc;
#endif
	else run_network = run_network_batch;

#if (EXECUTION_DRIVEN != 0)
	init_exd(fsin_cycle_relation, simics_cycle_relation, packet_size_in_phits,
		serv_addr, num_periodos_espera);
	run_network = run_network_exd;
#endif

	batch = alloc(sizeof(batch_t)*(samples+1));
	source_ports = alloc(sizeof(long)*n_ports);
	dest_ports = alloc(sizeof(long)*n_ports);
	port_utilization = alloc(sizeof(CLOCK_TYPE)*n_ports);

	if (plevel & 1) {
		destinations = alloc(sizeof(long *)*nprocs);
		sources = alloc(sizeof(long *)*nprocs);

		for (i=0; i<nprocs; i++) {
			destinations[i]=alloc(sizeof(long)*nprocs);
			sources[i]=alloc(sizeof(long)*nprocs);
			for (j=0; j<nprocs; j++){
				destinations[i][j]=0;
				sources[i][j]=0;
			}
		}
	}

	if (plevel & 4){
		if (topo<DIRECT)
			max_dst = nodes_x+nodes_y+nodes_z-2;
		else if (topo<=CUBE)
			max_dst = nodes_x+nodes_y+nodes_z;
		else if (topo<INDIRECT)
			max_dst = (2*nstages)+3;

		inj_dst = alloc(sizeof(long)*max_dst);
		con_dst = alloc(sizeof(long)*max_dst);
		for (i=0; i<max_dst; i++)
			inj_dst[i] = con_dst[i] = 0;
	}

	for (i=0; i<n_ports; i++) {
		source_ports[i]=0;
		dest_ports[i]=0;
		port_utilization[i]= (CLOCK_TYPE) 0L;
	}

	if (topo>DIRECT && routing<CUBE_ROUTING) {
		printf("WARNING: DOR only for direct topologies. Switching to Adaptive Routing\n");
		routing=ADAPTIVE_ROUTING;
	}

	switch (routing) {
		case DIMENSION_ORDER_ROUTING:
			check_rr = check_rr_dim_o_r;
			break;
		case DIRECTION_ORDER_ROUTING:
			check_rr = check_rr_dir_o_r;
			break;
		case STATIC_ROUTING:
		case ADAPTIVE_ROUTING:
			if (topo<DIRECT){
				printf("WARNING: For k-ary n-cubes routing should be DOR. Switching to Dimension Order Routing\n");
				check_rr = check_rr_dim_o_r;
			}
			break;
		case IC_1M_ROUTING:
			calc_rr=icube_1mesh_rr;
			break;
		case IC_4M_ROUTING:
            calc_rr=icube_4mesh_rr;
            break;
		default:
			panic("Bad routing order initializing");
	}

	switch (topo) {
		case MESH:
			neighbor = torus_neighbor;
			calc_rr = mesh_rr;
			break;
		case MIDIMEW:
			neighbor = midimew_neighbor;
			calc_rr = midimew_rr;
			break;
		case TORUS:
			neighbor = torus_neighbor;
			if (nways != 1) calc_rr = torus_rr;
			else calc_rr = torus_rr_unidir;
			break;
		case TWISTED:
			neighbor = dtt_neighbor;
			if (nways != 1) calc_rr = dtt_rr;
			else calc_rr = dtt_rr_unidir;
			break;
		case FATTREE:
			calc_rr = fattree_rr_adapt;
			if (routing==STATIC_ROUTING)
				check_rr = check_rr_fattree_static;
			else
				check_rr = check_rr_fattree_adaptive;
			break;
		case THINTREE:
			calc_rr = thintree_rr_adapt;
			if (routing==STATIC_ROUTING)
				check_rr = check_rr_thintree_static;
			else
				check_rr = check_rr_thintree_adaptive;
			break;
		case SLIMTREE:
			calc_rr = slimtree_rr_adapt;
			if (routing==STATIC_ROUTING)
				check_rr = check_rr_slimtree_static;
			else
				check_rr = check_rr_slimtree_adaptive;
			break;
		case ICUBE:
			if (routing==STATIC_ROUTING)
			{
				check_rr = check_rr_icube_static;
	   			calc_rr = icube_rr;
			}
			else if (routing==ADAPTIVE_ROUTING)
			{
				check_rr = check_rr_icube_adaptive;
				calc_rr = icube_rr;
			}
			else
			{
				check_rr = check_rr_icube_static_IB;
			}
			neighbor = torus_neighbor;
			break;
		default:
			panic("Bad topology initializing");
	}

	switch (req_mode) {
		case BUBBLE_OBLIVIOUS_REQ:
			request_port = request_port_bubble_oblivious;
			break;
		case BUBBLE_ADAPTIVE_RANDOM_REQ:
			request_port = request_port_bubble_adaptive_random;
			break;
		case BUBBLE_ADAPTIVE_SHORTEST_REQ:
			request_port = request_port_bubble_adaptive_shortest;
			break;
		case BUBBLE_ADAPTIVE_SMART_REQ:
			request_port = request_port_bubble_adaptive_smart;
			break;
		case DOUBLE_OBLIVIOUS_REQ:
			request_port = request_port_double_oblivious;
			break;
		case DOUBLE_ADAPTIVE_REQ:
			request_port = request_port_double_adaptive;
			break;
		case HEXA_OBLIVIOUS_REQ:
			request_port = request_port_hexa_oblivious;
			break;
		case HEXA_ADAPTIVE_REQ:
			request_port = request_port_hexa_adaptive;
			break;
		case DALLY_TRC_REQ:
			request_port = request_port_dally_trc;
			break;
		case DALLY_BASIC_REQ:
			request_port = request_port_dally_basic;
			break;
		case DALLY_IMPROVED_REQ:
			request_port = request_port_dally_improved;
			break;
		case DALLY_ADAPTIVE_REQ:
			request_port = request_port_dally_adaptive;
			break;
#if (BIMODAL_SUPPORT != 0)
		case BIMODAL_REQ:
			request_port = request_port_bimodal_random;
			break;
#endif /* BIMODAL */
		case TREE_REQ:
			request_port = request_port_tree;
			break;
		case ICUBE_REQ:
			if (routing==IC_1M_ROUTING || routing==IC_4M_ROUTING)
				request_port = request_port_icube_IB;
			else
				request_port = request_port_icube;
			break;
		default:
			panic("Bad routing strategy initializing");
	}

	switch (cons_mode) {
		case SINGLE_CONS:
			consume = consume_single;
			arbitrate_cons = arbitrate_cons_single;
			break;
		case MULTIPLE_CONS:
			consume = consume_multiple;
			arbitrate_cons = arbitrate_cons_multiple;
			break;
		default:
			panic("Bad consumption mode initializing");
	}

	switch (arb_mode) {
		case ROUNDROBIN_ARB:
			arbitrate_select = arbitrate_select_round_robin;
			break;
		case FIFO_ARB:
			arbitrate_select = arbitrate_select_fifo;
			break;
		case LONGEST_ARB:
			arbitrate_select = arbitrate_select_longest;
			break;
		case RANDOM_ARB:
			arbitrate_select = arbitrate_select_random;
			break;
		case AGE_ARB:
			arbitrate_select = arbitrate_select_age;
			break;
		default:
			panic("Bad arbitration mode initializing");
	}

	switch (inj_mode) {
		case SHORTEST_INJ:
			select_input_port = select_input_port_shortest;
			break;
		case DOR_INJ:
			select_input_port = select_input_port_dor_only;
			break;
		case DOR_SHORTEST_INJ:
			select_input_port = select_input_port_dor_shortest;
			break;
		case SHORTEST_PROFITABLE_INJ:
			select_input_port = select_input_port_shortest_profitable;
			break;
		case LONGEST_PATH_INJ:
			select_input_port = select_input_port_lpath;
			break;
		default:
			panic("Bad injection mode initializing");
	}

	if (topo<DIRECT)
	{
		data_movement = data_movement_direct;
		arbitrate = arbitrate_direct;
	}
	else 
	{
		data_movement = data_movement_indirect;
		if (topo==ICUBE)
			arbitrate = arbitrate_icube;
		else
		    arbitrate = arbitrate_trees;
	}
}
